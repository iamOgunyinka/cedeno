#include "candlestick_data.hpp"
#include "global_data.hpp"
#include "trades_data.hpp"
#include <algorithm>
#include <filesystem>

namespace backtesting {

::utils::waitable_container_t<kline_task_t> kline_task_t::klineScheduledTasks{};

binance_candlestick_data_t binance_candlestick_data_t::dataFromCSVStream(
    data_streamer_t<binance_candlestick_data_t> &dataStreamer) {
  binance_candlestick_data_t data;
  try {
    auto row = dataStreamer.getNextRow();
    if (row.empty())
      return binance_candlestick_data_t{};
    data = klineFromCSVRow(row);
  } catch (std::exception const &) {
  }

  return data;
}

binance_candlestick_data_t
binance_candlestick_data_t::klineFromCSVRow(csv::CSVRow const &row) {

  if (!isExpectedRowCount(row.size()))
    throw std::runtime_error("unexpected columns in a row, expects 16");

  binance_candlestick_data_t data{};
  data.intervalInSeconds = std::chrono::seconds(1); // column 3
  data.eventTime = 0;

  for (int i = 0; i < (int)row.size(); ++i) {
    auto const iter = (row.begin() + i);
    // all `time`s here are expected to be in milliseconds
    if (i == 0) {
      data.eventTime = getTimeInSecs<uint64_t>(iter);
    } else if (i == 1) {
      data.startTime = getTimeInSecs<uint64_t>(iter);
    } else if (i == 2) {
      data.closeTime = getTimeInSecs<uint64_t>(iter);
    } else if (i == 4) {
      data.firstTradeID = getNumber<uint64_t>(iter);
    } else if (i == 5) {
      data.lastTradeID = getNumber<uint64_t>(iter);
    } else if (i == 6) {
      data.openPrice = getNumber<double>(iter);
    } else if (i == 7) {
      data.closePrice = getNumber<double>(iter);
    } else if (i == 8) {
      data.highPrice = getNumber<double>(iter);
    } else if (i == 9) {
      data.lowPrice = getNumber<double>(iter);
    } else if (i == 10) {
      data.baseAssetVolume = getNumber<double>(iter);
    } else if (i == 11) {
      data.numberOfTrades = getNumber<int>(iter);
    } else if (i == 12) {
      data.klineIsClosed = getNumber<int>(iter);
    } else if (i == 13) {
      data.quoteAssetVolume = getNumber<double>(iter);
    } else if (i == 14) {
      data.tbBaseAssetVolume = getNumber<double>(iter);
    } else if (i == 15) {
      data.tbQuoteAssetVolume = getNumber<double>(iter);
    }
  }
  return data;
}

kline_data_t binanceKlineToLocalKline(binance_candlestick_data_t const &data) {
  kline_data_t result;
  result.ts = data.eventTime;
  result.openPrice = data.openPrice;
  result.highPrice = data.highPrice;
  result.lowPrice = data.lowPrice;
  result.closePrice = data.closePrice;
  result.baseVolume = data.baseAssetVolume;
  result.quoteVolume = data.quoteAssetVolume;
  return result;
}

kline_data_t &kline_data_t::operator+=(kline_data_t const &data) {
  if (this == &data)
    return *this;
  if (data.ts <= ts)
    openPrice = data.openPrice;
  else
    closePrice = data.closePrice;

  ts = (std::min)(data.ts, ts);
  highPrice = (std::max)(data.highPrice, highPrice);
  lowPrice = (std::min)(data.lowPrice, lowPrice);
  baseVolume += data.baseVolume;
  quoteVolume += data.quoteVolume;
  ++ntrades;

  return *this;
}

bool checkAndValidateKlineRequest(kline_config_t &config) {
  if (config.interval < data_interval_e::one_second ||
      config.interval > data_interval_e::one_month)
    return false;
  auto &allTradableTokens = global_data_t::instance().allTokens;
  config.symbol = utils::toUpperString(config.symbol);
  auto iter =
      std::lower_bound(allTradableTokens.begin(), allTradableTokens.end(),
                       config, [](auto const &a, auto const &b) {
                         using T = std::remove_cv_t<std::decay_t<decltype(a)>>;
                         if constexpr (std::is_same_v<T, kline_config_t>) {
                           return std::make_tuple(a.symbol, a.tradeType) <
                                  std::make_tuple(b.name, b.tradeType);
                         } else {
                           return std::make_tuple(a.name, a.tradeType) <
                                  std::make_tuple(b.symbol, b.tradeType);
                         }
                       });
  if (iter == allTradableTokens.end() ||
      !utils::isCaseInsensitiveStringCompare(iter->name, config.symbol))
    return false;
  if (config.endTime == 0)
    config.endTime = std::time(nullptr);
  if (config.startTime == 0)
    config.startTime = config.endTime - 3'600;
  config.limit = std::clamp(config.limit, (int16_t)0, (int16_t)1'000);
  config.tradeType = iter->tradeType;
  return true;
}

optional_kline_list_t getDiscreteKlineData(kline_config_t &&config) {
  if (!checkAndValidateKlineRequest(config))
    return std::nullopt;

  auto const filePaths = utils::listOfFilesForTradeData(
      config.startTime, config.endTime, config.tradeType, config.symbol,
      CANDLESTICK);
  if (filePaths.empty())
    return std::nullopt;

  backtesting::data_streamer_t<binance_candlestick_data_t> dataStream(
      filePaths);

  kline_data_list_t result;
  for (binance_candlestick_data_t temp = dataStream.getNextData();
       temp.eventTime != 0; temp = dataStream.getNextData()) {
    if (result.empty()) {
      result.push_back(binanceKlineToLocalKline(temp));
      temp = dataStream.getNextData();
      continue;
    }

    auto newData = binanceKlineToLocalKline(temp);
    auto &lastData = result.back();
    if (requiresNewInsertion(newData.ts, lastData.ts, config.interval,
                             time_duration_e::seconds)) {
      result.push_back(std::move(newData));
    } else {
      lastData += newData;
    }
  }

  return result;
}

bool getContinuousKlineData(kline_config_t &&config) {
  if (!checkAndValidateKlineRequest(config))
    return false;

  auto filePaths = utils::listOfFilesForTradeData(
      config.startTime, config.endTime, config.tradeType, config.symbol,
      CANDLESTICK);
  if (filePaths.empty())
    return false;

  data_streamer_t<binance_candlestick_data_t> dataStream(std::move(filePaths));
  kline_task_t rt(std::move(dataStream));
  kline_task_t::klineScheduledTasks.append(std::move(rt));
  return true;
}

void candlestickProcessingImpl() {
  int threadsAtWork = 0;
  while (true) {
    auto task = kline_task_t::klineScheduledTasks.get();
    if (threadsAtWork > 4) // ignore
      continue;

    ++threadsAtWork;
    std::thread([t = std::move(task), &threadsAtWork]() mutable {
      klineChildThreadImpl(std::move(t));
      --threadsAtWork;
    }).detach();
  }
}

void klineChildThreadImpl(kline_task_t &&task) {
  binance_candlestick_data_t temp = task.dataStream.getNextData();
  while (temp.eventTime != 0) {
    task.callback({binanceKlineToLocalKline(temp)});
    time_t const currentTime = temp.eventTime;
    temp = task.dataStream.getNextData();
    if (temp.eventTime == 0)
      return;
    auto const interval = temp.eventTime - currentTime;
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  }
}

namespace utils {
std::vector<std::filesystem::path>
listOfFilesForTradeData(time_t const startTime, time_t const endTime,
                        trade_type_e const tt, std::string const &symbol,
                        std::string const &stream) {
  std::filesystem::path const rootPath(global_data_t::instance().rootPath);
  std::vector<std::time_t> intervals =
      utils::intervalsBetweenDates(startTime, endTime);
  if (intervals.empty())
    return {};

  if (intervals.back() < endTime)
    intervals.push_back(endTime);

  auto const tradeType = tt == trade_type_e::spot ? SPOT : FUTURES;
  std::set<std::filesystem::path> filePaths;

  auto const startDate = intervals.front();
  auto const endDate = intervals.back();

  for (auto const &interval : intervals) {
    auto const dateString = utils::currentTimeToString(interval, "_").value();
    auto const path = rootPath / symbol / dateString / stream / tradeType;
    if (!std::filesystem::exists(path))
      continue;

    time_t startOfThatDay =
        utils::dateStringToTimeT(
            utils::currentTimeToString(startDate, "-").value() + " 00:00:00")
            .value();
    for (auto const &fileEntry :
         std::filesystem::recursive_directory_iterator(path)) {
      std::filesystem::path const file = fileEntry.path();
      if (!std::filesystem::is_regular_file(file))
        continue;
      //  we need the filename without the .csv extension
      std::string const &baseName = file.stem().string();
      time_t const fileDateTime =
          startOfThatDay + utils::timeStringToSeconds(baseName);
      if (startDate >= fileDateTime && endDate <= fileDateTime)
        filePaths.insert(file);
    }
  }
  return std::vector<std::filesystem::path>(filePaths.begin(), filePaths.end());
}
} // namespace utils
// namespace utils
} // namespace backtesting
