#include "bookticker.hpp"
#include "global_data.hpp"
#include "trades_data.hpp"
#include <algorithm>
#include <filesystem>

namespace backtesting {

::utils::waitable_container_t<bktick_config_t>
    bktick_config_t::bookTickerConfigs{};

std::array<std::vector<bktick_data_t>, 2>
    bktick_config_t::globalBkTickerRecord{};

std::mutex bktick_config_t::globalBkTickerMutex{};

binance_bktick_data_t binance_bktick_data_t::dataFromCSVStream(
    data_streamer_t<binance_bktick_data_t> &dataStreamer) {
  binance_bktick_data_t data;
  try {
    auto row = dataStreamer.getNextRow();
    if (row.empty())
      return binance_bktick_data_t{};
    data = bookTickerFromCSVRow(row);
  } catch (std::exception const &) {
  }

  return data;
}

binance_bktick_data_t
binance_bktick_data_t::bookTickerFromCSVRow(csv::CSVRow const &row) {
  if (!isExpectedRowCount(row.size()))
    throw std::runtime_error("unexpected columns in a row, expects 7");
  auto iter = row.begin();

  binance_bktick_data_t data{};
  data.orderBookUpdateID = getNumber<double>(++iter);
  data.bestBidPrice = getNumber<double>(++iter);
  data.bestBidQty = getNumber<double>(++iter);
  data.bestAskPrice = getNumber<double>(++iter);
  data.bestAskQty = getNumber<double>(++iter);
  data.eventTimeInMs = (std::max)(getNumber<uint64_t>(++iter), (uint64_t)1'000);
  data.transactionTimeInMs =
      (std::max)(getNumber<uint64_t>(++iter), (uint64_t)1'000);
  return data;
}

bktick_data_t binanceBTickerToLocalBTicker(binance_bktick_data_t const &data) {
  bktick_data_t result;
  result.ts = data.eventTimeInMs;
  result.bestBidPrice = data.bestBidPrice;
  result.bestBidQty = data.bestBidQty;
  result.bestAskPrice = data.bestAskPrice;
  result.bestAskQty = data.bestAskQty;
  return result;
}

bool requiresNewInsertionMs(uint64_t const &fresh, uint64_t const &old,
                            data_interval_e const interval) {
  auto const diff =
      std::chrono::milliseconds(old) - std::chrono::milliseconds(fresh);

  switch (interval) {
  case data_interval_e::one_second:
    return diff > std::chrono::seconds(1);
  case data_interval_e::one_minute:
    return diff > std::chrono::minutes(1);
  case data_interval_e::three_minutes:
    return diff > std::chrono::minutes(3);
  case data_interval_e::five_minutes:
    return diff > std::chrono::minutes(5);
  case data_interval_e::fifteen_minutes:
    return diff > std::chrono::minutes(15);
  case data_interval_e::thirty_minutes:
    return diff > std::chrono::minutes(30);
  case data_interval_e::one_hour:
    return diff > std::chrono::hours(1);
  case data_interval_e::two_hours:
    return diff > std::chrono::hours(2);
  case data_interval_e::four_hours:
    return diff > std::chrono::hours(4);
  case data_interval_e::six_hours:
    return diff > std::chrono::hours(6);
  case data_interval_e::twelve_hours:
    return diff > std::chrono::hours(12);
  case data_interval_e::one_day:
    return diff > std::chrono::hours(24);
  case data_interval_e::three_days:
    return diff > std::chrono::hours(72);
  case data_interval_e::one_week:
    return diff > std::chrono::hours(24 * 7);
  case data_interval_e::one_month:
    return diff > std::chrono::hours(24 * 28);
  }
  return false;
}

bool requiresNewInsertion(uint64_t const &fresh, uint64_t const &old,
                          data_interval_e const interval,
                          time_duration_e const durationSpec) {
  if (durationSpec == time_duration_e::milliseconds)
    return requiresNewInsertionMs(fresh, old, interval);

  uint64_t const diff = std::abs((double)old - (double)fresh);
  switch (interval) {
  case data_interval_e::one_second:
    return diff > 1'000;
  case data_interval_e::one_minute:
    return diff > 60'000;
  case data_interval_e::three_minutes:
    return diff > 180'000;
  case data_interval_e::five_minutes:
    return diff > 300'000;
  case data_interval_e::fifteen_minutes:
    return diff > (60'000 * 15);
  case data_interval_e::thirty_minutes:
    return diff > (60'000 * 30);
  case data_interval_e::one_hour:
    return diff > (360'000);
  case data_interval_e::two_hours:
    return diff > (720'000);
  case data_interval_e::four_hours:
    return diff > (360'000 * 4);
  case data_interval_e::six_hours:
    return diff > (360'000 * 6);
  case data_interval_e::twelve_hours:
    return diff > (360'000 * 12);
  case data_interval_e::one_day:
    return diff > (360'000 * 24);
  case data_interval_e::three_days:
    return diff > (360'000 * 24 * 3);
  case data_interval_e::one_week:
    return diff > (360'000 * 24 * 7);
  case data_interval_e::one_month:
    return diff > (360'000 * 24 * 28);
  }
  return false;
}

bool checkAndValidateBookTickerRequestHelper(bktick_config_t &config) {
  auto &allTradableTokens = global_data_t::instance().allTokens;
  config.symbols[0] = utils::toUpperString(config.symbols[0]);
  auto iter =
      std::lower_bound(allTradableTokens.cbegin(), allTradableTokens.cend(),
                       config, [](auto const &a, auto const &b) {
                         using T = std::remove_cv_t<std::decay_t<decltype(a)>>;
                         if constexpr (std::is_same_v<T, bktick_config_t>) {
                           return std::make_tuple(a.symbols[0], a.tradeType) <
                                  std::make_tuple(b.name, b.tradeType);
                         } else {
                           return std::make_tuple(a.name, a.tradeType) <
                                  std::make_tuple(b.symbols[0], b.tradeType);
                         }
                       });
  if (iter == allTradableTokens.cend() ||
      !utils::isCaseInsensitiveStringCompare(iter->name, config.symbols[0]))
    return false;

  if (config.callback) {
    if (config.endTime == 0)
      config.endTime = std::time(nullptr);
    if (config.startTime == 0)
      config.startTime = config.endTime - 3'600;
  }
  config.tradeType = iter->tradeType;
  return true;
}

bool checkAndValidateBookTickerRequest(bktick_config_t &config) {
  auto const size = config.symbols.size();
  if (size == 0)
    return false;
  for (auto const &c : config.symbols) {
    auto newConfig = config;
    newConfig.symbols = {c};
    if (!checkAndValidateBookTickerRequestHelper(newConfig))
      return false;
  }
  return true;
}

void insertNewestData(bktick_data_t const &data,
                      bktick_config_t const &config) {
  size_t const index = config.tradeType == trade_type_e::spot ? 0 : 1;

  // this function will be called from several threads, so we need to protect it
  std::lock_guard<std::mutex> mLock(bktick_config_t::globalBkTickerMutex);
  auto &dataList = bktick_config_t::globalBkTickerRecord[index];
  if (dataList.empty())
    return dataList.push_back(data);

  // list is meant to be small, so no need for std::algorithms
  for (size_t i = 0; i < dataList.size(); ++i) {
    if (utils::isCaseInsensitiveStringCompare(dataList[i].symbol,
                                              config.symbols[0])) {
      dataList[i] = data;
      return;
    }
  }
  // if we're here, data is not in the list
  dataList.push_back(data);
}

bool getContinuousBTickerData(bktick_config_t &&config) {
  if (!checkAndValidateBookTickerRequest(config))
    return false;

  for (auto const &c : config.symbols) {
    auto newConfig = config;
    newConfig.symbols = {c};
    bktick_config_t::bookTickerConfigs.append(std::move(newConfig));
  }
  return true;
}

bktick_list_t getDiscreteBTickerData(bktick_config_t &&config) {
  if (!checkAndValidateBookTickerRequest(config))
    return {};

  size_t const index = config.tradeType == trade_type_e::spot ? 0 : 1;
  bktick_list_t result;
  for (auto const &symbol : config.symbols) {
    std::lock_guard<std::mutex> lockG(bktick_config_t::globalBkTickerMutex);
    auto &dataList = bktick_config_t::globalBkTickerRecord[index];
    auto iter = std::find_if(
        dataList.cbegin(), dataList.cend(), [symbol](bktick_data_t const &c) {
          return utils::isCaseInsensitiveStringCompare(symbol, c.symbol);
        });
    if (iter != dataList.cend())
      result.push_back(*iter);
  }
  return result;
}

void bookTickerChildThreadImpl(bktick_config_t &&config) {
  auto filePaths = utils::listOfFilesForTradeData(
      config.startTime, config.endTime, config.tradeType, config.symbols[0],
      CANDLESTICK);
  if (filePaths.empty())
    return;

  data_streamer_t<binance_bktick_data_t> dataStream(std::move(filePaths));

  auto const &symbol = config.symbols[0];
  auto data = dataStream.getNextData();
  while (data.eventTimeInMs > 0) {
    auto newestData = binanceBTickerToLocalBTicker(data);
    newestData.symbol = symbol;
    config.callback(newestData);
    insertNewestData(newestData, config); // needed for discrete bookTicker

    auto const lastTime = data.eventTimeInMs;
    data = dataStream.getNextData();
    auto timeToWait = data.eventTimeInMs - lastTime;
    if (timeToWait == 0)
      timeToWait = 1'000;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeToWait));
  }
}

void bookTickerProcessingThreadImpl() {
  bktick_config_t::globalBkTickerRecord[0] = {}; // spot
  bktick_config_t::globalBkTickerRecord[1] = {}; // futures

  int threadsAtWork = 0;
  std::condition_variable cv{};
  std::mutex taskWaitMutex{};

  while (true) {
    auto config = bktick_config_t::bookTickerConfigs.get();

    if (config.symbols.empty())
      continue;
    if (config.symbols.size() != 1)
      config.symbols.resize(1);

    if (threadsAtWork > 4) {
      std::unique_lock<std::mutex> uLock{taskWaitMutex};
      cv.wait(uLock, [threadsAtWork] { return threadsAtWork < 4; });
      if (threadsAtWork >= 4) // ignore and continue -> an error
        continue;
    }

    ++threadsAtWork;
    std::thread([&, c = std::move(config)]() mutable {
      bookTickerChildThreadImpl(std::move(c));
      std::lock_guard<std::mutex> myLock(taskWaitMutex);
      --threadsAtWork;
      cv.notify_one();
    }).detach();
  }
}

} // namespace backtesting
