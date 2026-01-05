#include "Indicators.h"

std::vector<double> calculateEMA(const std::vector<double>& data, int period)
{
      // Заполняем результат NaN-ами до расчёта
    //cout << "data.size(): " << data.size() << endl;
    vector<double> EMPTYEMA;
    if (data.size() < period || period <= 0) {
        std::cerr << "EMA: Not enough data or invalid period!\n";
        return EMPTYEMA;
    }
    else if (data.size() >= period)
    {
        std::vector<double> ema(data.size(), std::nan(""));
        double multiplier = 2.0 / (period + 1);
        double sma = 0.0;

        // Сначала считаем простую скользящую среднюю (SMA) для первых period элементов
        for (int i = 0; i < period; ++i) {
            sma += data[i];
        }
        sma /= period;
        ema[period - 1] = sma;

        // Далее считаем EMA
        for (size_t i = period; i < data.size(); ++i) {
            ema[i] = (data[i] - ema[i - 1]) * multiplier + ema[i - 1];
        }
        return ema;
    }
    return EMPTYEMA;
}

std::vector<double> calculateNATR(const std::vector<OHLC>& data, int period)
{
      // Инициализируем NaN для всех элементов
	vector<double> EMPTYNATR;
    if (data.size() < period || period <= 0) {
        std::cerr << "NATR: Not enough data or invalid period!\n";
        return EMPTYNATR;
    }
    if (data.size() >= period)
    {
        std::vector<double> natr(data.size(), std::nan(""));
        std::vector<double> tr_values(data.size(), 0.0);  // Массив для хранения True Range

        // Рассчитываем True Range для каждого дня
        for (size_t i = 1; i < data.size(); ++i) {
            double tr = std::max({ data[i].high - data[i].low,
                                  std::abs(data[i].high - data[i - 1].close),
                                  std::abs(data[i].low - data[i - 1].close) });
            tr_values[i] = tr;
        }

        // Считаем ATR для первого "period" элементов
        double atr = 0.0;
        for (int i = 1; i < period; ++i) {
            atr += tr_values[i];
        }
        atr /= period;

        // Далее считаем нормализованный ATR (NATR)
        for (size_t i = period; i < data.size(); ++i) {
            if (data[i].close != 0) {  // Чтобы избежать деления на ноль
                natr[i] = (atr / data[i].close) * 100;
            }
            // Обновляем ATR по схеме экспоненциального сглаживания (EMA)
            atr = ((atr * (period - 1)) + tr_values[i]) / period;
        }

        return natr;
    }
	return EMPTYNATR;
}


vector<double> TA_SMA(const std::vector<double>& data, int period)
{
    vector<double> EMPTYSMA;
    if (data.size() >= period)
    {
        
        TA_Initialize();

        // Prepare output vector for the SMA results (must match input data size)
        std::vector<double> smaResult(data.size(), nan(""));

        // Calculate the SMA using TA-Lib's TA_SMA function
        int begin, length;
        TA_RetCode retCode = TA_SMA(0, data.size() - 1, &data[0], period, &begin, &length, &smaResult[0]);

        // Check for errors
        if (retCode != TA_SUCCESS) {
            std::cerr << "Error calculating SMA: " << retCode << std::endl;
            TA_Shutdown(); // Shutdown TA-Lib after use
            return EMPTYSMA;
        }


        // Shutdown TA-Lib after use

        TA_Shutdown();
		vector<double> ReverseResult(smaResult.rbegin(), smaResult.rend()); // Reverse the result to match the original order
        return ReverseResult;
    }
    return EMPTYSMA;
}
