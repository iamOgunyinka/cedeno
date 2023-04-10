#pragma once

#include <csv.hpp>
#include <filesystem>
#include <queue>

namespace backtesting {

using fs_list_t = std::vector<std::filesystem::path>;

template <typename T> struct data_streamer_t {
  data_streamer_t(fs_list_t const &fileNames) {
    m_paths.reserve(fileNames.size());
    for (auto const &filename : fileNames)
      m_paths.push_back({filename.string(), false});
  }

  ~data_streamer_t() {
    if (m_csvReader)
      delete m_csvReader;
    m_csvReader = nullptr;
  }

  T getNextData() {
    if (!m_unusedTData.empty()) {
      auto data = m_unusedTData.front();
      m_unusedTData.pop();
      return data;
    }

    if (!m_csvReader) {
      auto currentFile = getNextUnreadFile();
      if (!currentFile)
        throw std::runtime_error("no file to open");
      m_csvReader = new csv::CSVReader(currentFile->filename);
      currentFile->isOpened = true;
    }

    assert(m_csvReader != nullptr);
    return T::dataFromCSVStream(*this);
  }

  inline void putBack(csv::CSVRow &&row) {
    m_lastUnusedData.push(std::move(row));
  }

  inline void putBack(T &&data) { m_unusedTData.emplace(std::move(data)); }

  csv::CSVRow getNextRow() {
    if (!m_lastUnusedData.empty()) {
      auto front = m_lastUnusedData.front();
      m_lastUnusedData.pop();
      return front;
    }

    if (!m_csvReader || (!m_firstTimeRead && m_csvReader->empty())) {
      if (auto currentFile = getNextUnreadFile(); currentFile) {
        if (m_csvReader)
          delete m_csvReader;
        m_csvReader = new csv::CSVReader(currentFile->filename);
        currentFile->isOpened = true;
      } else {
        if (m_csvReader && m_csvReader->empty())
          return {};
        throw std::runtime_error("no file to open");
      }
    }

    if (m_firstTimeRead)
      m_firstTimeRead = false;

    csv::CSVRow row;
    if (!m_csvReader->read_row(row) && m_csvReader->empty())
      return getNextRow();
    return row;
  }

private:
  struct internal_data_t {
    std::string filename;
    bool isOpened;
  };

  internal_data_t *getNextUnreadFile() {
    for (internal_data_t &file : m_paths) {
      if (!file.isOpened)
        return &file;
    }
    return nullptr;
  }

  csv::CSVReader *m_csvReader = nullptr;
  std::vector<internal_data_t> m_paths;
  std::queue<csv::CSVRow> m_lastUnusedData;
  std::queue<T> m_unusedTData;
  bool m_firstTimeRead = true;
};

} // namespace backtesting
