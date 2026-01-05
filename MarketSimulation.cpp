// MarketSimulation.cpp : Defines the entry point for the application.
//

#include "MarketSimulation.h"
using namespace MarketSimulation;


//extern "C" uint MarketSimulation::CountID;
uint MarketSimulation::Account::OpenOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, Market& Market, const char* NameInstrument)
{
	// Logic to open an order
	// This could involve checking balance, instrument availability, etc.
	if (Volume <= 0 || TakeProfit < 0 || StopLoss < 0) {
#ifdef DEBUG
		CoutASSERT(Volume > 0 || TakeProfit > 0 || StopLoss > 0, "Invalid parameters for opening order.");
#endif
		//throw std::runtime_error("Invalid parameters for opening order.");
		return false; // Invalid parameters
	}
	ResultOrder result = Market.GetRequestOpenOrder(Volume, TypeOrder, TakeProfit, StopLoss, NameInstrument, _NameAccount);
	if (result.IsSuccess) {
		// Add to current orders
		TableCurrentOrders currentOrder;
		currentOrder.NameInstrument = result.NameInstrument; // Set the instrument name
		currentOrder.OpenTime = result.OpenTime;
		currentOrder.OpenPrice = result.OpenPrice;
		currentOrder.StopLoss = result.StopLoss;
		currentOrder.TakeProfit = result.TakeProfit;
		currentOrder.Volume = result.Volume;
		currentOrder.TypeOrder = result.TypeOrder;
		currentOrder.ID = result.ID;
		_CurrentOrders.push_back(currentOrder);
		double VolumeCost = result.Volume * result.OpenPrice; // Calculate the cost of the order
		TakeAwayBalance(VolumeCost); // Deduct the volume cost from balance
		return result.ID;
	}
	return false;
}

uint MarketSimulation::Account::OpenLimitOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, Market& Market, const char* NameInstrument, double LimitOpenPrice, long long TimeExpiration)
{
	// Logic to open an order
	// This could involve checking balance, instrument availability, etc.
	if (Volume <= 0 || TakeProfit < 0 || StopLoss < 0) {
#ifdef DEBUG
		CoutASSERT(Volume > 0 || TakeProfit > 0 || StopLoss > 0, "Invalid parameters for opening order.");
		//throw std::runtime_error("Invalid parameters for opening order.");
#endif
		return false; // Invalid parameters
	}
	if (LimitOpenPrice <= 0) {
#ifdef DEBUG
		CoutASSERT(LimitOpenPrice > 0, "Limit open price must be greater than zero.");
		//throw std::runtime_error("Limit open price must be greater than zero.");
#endif
		return false; // Invalid limit open price
	}
	if (TypeOrder != BUY_LIMIT_ORDER && TypeOrder != SELL_LIMIT_ORDER) {
#ifdef DEBUG
		CoutASSERT(false, "Invalid order type for limit order.");
#endif
		//throw std::runtime_error("Invalid order type for limit order.");
		return false; // Invalid order type
	}
	ResultOrder result = Market.GetRequestOpenLimitOrder(Volume, TypeOrder, TakeProfit, StopLoss, NameInstrument, _NameAccount, LimitOpenPrice, TimeExpiration);
	if (result.IsSuccess) {
		// Add to current orders
		TableCurrentOrders currentOrder;
		currentOrder.NameInstrument = result.NameInstrument; // Set the instrument name
		currentOrder.OpenTime = 0;
		currentOrder.OpenPrice = 0;
		currentOrder.StopLoss = result.StopLoss;
		currentOrder.TakeProfit = result.TakeProfit;
		currentOrder.Volume = result.Volume;
		currentOrder.TypeOrder = TypeOrder;
		currentOrder.LimitOpenPrice = LimitOpenPrice; // Set the limit open price
		currentOrder.TimeExpiration = TimeExpiration; // Set the time expiration for the limit order
		currentOrder.ID = result.ID;
		_CurrentOrders.push_back(currentOrder);
		double VolumeCost = result.Volume * result.OpenPrice; // Calculate the cost of the order
		TakeAwayBalance(VolumeCost); // Deduct the volume cost from balance
		return result.ID;
	}
	return false;
}

bool MarketSimulation::Account::CloseOrder(double Volume, uint TypeOrder, const char* NameInstrument, Market& Market)
{
	ResultOrder result = Market.GetRequestCloseOrder(Volume, TypeOrder, NameInstrument, _NameAccount);
	if (!result.IsSuccess) {
#ifdef DEBUG
		CoutASSERT(false, "Failed to close order.");
#endif
		return false; // Order closing failed
	}
	for (int i = 0; i < _CurrentOrders.size(); i++) {
		if (_CurrentOrders[i].NameInstrument == result.NameInstrument && _CurrentOrders[i].TypeOrder == TypeOrder, _CurrentOrders[i].Volume >= Volume) {
			// Remove the order from current orders
			if (_CurrentOrders[i].Volume == Volume) {
				// If the volume is equal to the current order volume, remove the order completely
				double AddMoney = 0;
				if (TypeOrder == BUY_ORDER || TypeOrder == BUY_LIMIT_ORDER)
				{
					//CoutASSERT(_CurrentOrders[i].OpenPrice > 0, "Open price must be greater than zero for BUY order.");
					AddMoney = result.ClosePrice * Volume;
				}
				else if (TypeOrder == SELL_ORDER || TypeOrder == SELL_LIMIT_ORDER)
				{
					//CoutASSERT(_CurrentOrders[i].OpenPrice < 0, "Open price must be less than zero for SELL order.");
					AddMoney = (result.OpenPrice * Volume - result.ClosePrice * Volume) + result.ClosePrice * Volume; // Adjust for SELL order
				}
				//double AddMoney = result.ClosePrice * Volume;
				SetBalance(AddMoney); // Add the closed order amount to the balance
				TableHistoricalOrders historicalOrder;
				historicalOrder.OpenTime = _CurrentOrders[i].OpenTime;
				historicalOrder.OpenPrice = _CurrentOrders[i].OpenPrice;
				historicalOrder.ClosePrice = result.ClosePrice; // Assuming ClosePrice is available in ResultOrder
				historicalOrder.StopLoss = result.StopLoss;
				historicalOrder.TakeProfit = result.TakeProfit; // Assuming no SL/TP for historical orders
				historicalOrder.Volume = Volume;
				historicalOrder.TypeOrder = TypeOrder; // Assuming TypeOrder is defined somewhere
				historicalOrder.TimeExpiration = 0;
				_HistoricalOrders.push_back(historicalOrder); // Add to historical orders
				_CurrentOrders.erase(_CurrentOrders.begin() + i);
				return true;
			}
			else if (_CurrentOrders[i].Volume > Volume) {
				// If the volume is greater than the current order volume, reduce the volume
				double AddMoney = result.ClosePrice * Volume;
				SetBalance(AddMoney); // Add the closed order amount to the balance
				TableHistoricalOrders historicalOrder;
				historicalOrder.OpenTime = _CurrentOrders[i].OpenTime;
				historicalOrder.OpenPrice = _CurrentOrders[i].OpenPrice;
				historicalOrder.ClosePrice = result.ClosePrice; // Assuming ClosePrice is available in ResultOrder
				historicalOrder.StopLoss = result.StopLoss;
				historicalOrder.TakeProfit = result.TakeProfit; // Assuming no SL/TP for historical orders
				historicalOrder.Volume = Volume;
				historicalOrder.TypeOrder = TypeOrder;
				historicalOrder.ID = result.ID;// Assuming TypeOrder is defined somewhere
				_HistoricalOrders.push_back(historicalOrder); // Add to historical orders
				return true;
			}
		}
	}
	return false;
}


ResultOrder MarketSimulation::Market::GetRequestOpenOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, const char* NameInstrument, string& AccountName)
{
	// Logic to process open order requests
	// This could involve checking account balance, instrument availability, etc.
	ResultOrder result;
	int IndexInstrument = -1;
	if (MarketIsStarted)
	{
		if (Volume <= 0 || TakeProfit < 0 || StopLoss < 0) {
#ifdef DEBUG
			CoutASSERT(Volume > 0 || TakeProfit > 0 || StopLoss > 0, "Invalid parameters for opening order.");
#endif
			result.IsSuccess = false;
			return result; // Invalid parameters
		}
		if (_CurrentCandlesData == nullptr)
		{
#ifdef DEBUG
			CoutASSERT(false, "Current candles data is empty, cannot open order.");
#endif
			result.IsSuccess = false;
			return result; // No current candles data available
		}
		string StrNameInstrument(NameInstrument);
		vector<DataFile> *CurrentData = &FindCurrentCandlesData(StrNameInstrument);
		if (CurrentData->empty())
		{
#ifdef DEBUG
			CoutASSERT(!CurrentData->empty(), "Vector of CurrentCandles is empty!");
#endif
			result.IsSuccess = false;
			return result;
		}
		for (Account* Acc : _Accounts)
		{
			if (Acc->GetNameAccount() == AccountName && Acc->IsEnoughMoney(Volume * CurrentData->back().OpenPrice))
			{
				// Check if the instrument exists in the market
				for (int i = 0; i < _Instruments.size(); i++)
				{
					if (_Instruments[i].GetNameInstrument() == NameInstrument)
					{
						result.NameInstrument = NameInstrument;
						IndexInstrument = i; // Store the index of the instrument
						break;
					}
				}
				if (result.NameInstrument.empty())
				{
#ifdef DEBUG
					CoutASSERT(false, "Instrument not found in market.");
#endif
					result.IsSuccess = false;
					return result; // Instrument not found
				}


				result.IsSuccess = true;
				vector<DataFile> CurrentCandels = FindCurrentCandlesData(result.NameInstrument);
				DataFile dataFile = CurrentCandels[CurrentCandels.size() - 1]; // Get the latest candle data
				result.OpenTime = dataFile.OpenTime; // Current time as open time
				result.OpenPrice = dataFile.OpenPrice;
				result.StopLoss = StopLoss;
				result.TakeProfit = TakeProfit;
				result.Volume = Volume;
				result.ClosePrice = 0.0;
				result.TypeOrder = TypeOrder;
				result.ID = CountID;
				CountID++;
				TableActives active = {
					.NameActive = result.NameInstrument, //Require to correct this code BTC or USDT
					.Volume = Volume,
					.AccountName = AccountName,
					.OpenPrice = result.OpenPrice,
					.OpenTime = result.OpenTime,
					.TakeProfit = result.TakeProfit,
					.StopLoss = result.StopLoss,
					.TypeOrder = result.TypeOrder,
					.ID = result.ID
				};
				_TableActives.push_back(active); // Add to active trades table
			}
			else
			{
#ifdef DEBUG
				CoutASSERT(MarketIsStarted == true, "Market is not started!");
#endif
				result.IsSuccess = false;
			}
			}
		}
		
	
	return result;
}

ResultOrder MarketSimulation::Market::GetRequestOpenLimitOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, const char* NameInstrument, string& AccountName, double LimitOpenPrice, long long TimeExpiration)
{
	// Logic to process open order requests
	// This could involve checking account balance, instrument availability, etc.
	ResultOrder result;
	int IndexInstrument = -1;
	if (MarketIsStarted)
	{
		if (Volume <= 0 || TakeProfit < 0 || StopLoss < 0) {
#ifdef DEBUG
			CoutASSERT(Volume > 0 || TakeProfit > 0 || StopLoss > 0, "Invalid parameters for opening order.");
#endif
			result.IsSuccess = false;
			return result; // Invalid parameters
		}
		if (_CurrentCandlesData == nullptr)
		{
#ifdef DEBUG
			CoutASSERT(false, "Current candles data is empty, cannot open order.");
#endif
			result.IsSuccess = false;
			return result; // No current candles data available
		}
		if (LimitOpenPrice <= 0) {
#ifdef DEBUG
			CoutASSERT(LimitOpenPrice > 0, "Limit open price must be greater than zero.");
#endif
			result.IsSuccess = false;
			return result; // Invalid limit open price
		}

		string StrNameInstrument(NameInstrument);
		vector<DataFile>* CurrentData = &FindCurrentCandlesData(StrNameInstrument);
		if (CurrentData->empty())
		{
#ifdef DEBUG
			CoutASSERT(!CurrentData->empty(), "Vector of CurrentCandles is empty!");
#endif
			result.IsSuccess = false;
			return result;
		}
		for (Account* Acc : _Accounts)
		{
			if (Acc->GetNameAccount() == AccountName && Acc->IsEnoughMoney(Volume * CurrentData->back().OpenPrice))
			{
				// Check if the instrument exists in the market
				for (int i = 0; i < _Instruments.size(); i++)
				{
					if (_Instruments[i].GetNameInstrument() == NameInstrument)
					{
						result.NameInstrument = NameInstrument;
						IndexInstrument = i; // Store the index of the instrument
						break;
					}
				}
				if (result.NameInstrument.empty())
				{
#ifdef DEBUG
					CoutASSERT(false, "Instrument not found in market.");
#endif
					result.IsSuccess = false;
					return result; // Instrument not found
				}


				result.IsSuccess = true;
				vector<DataFile> CurrentCandels = FindCurrentCandlesData(result.NameInstrument);
				DataFile dataFile = CurrentCandels[CurrentCandels.size() - 1]; // Get the latest candle data
				if (dataFile.OpenTime >= TimeExpiration)
				{
#ifdef DEBUG
					CoutASSERT(false, "Invalid Time Expiration!");
#endif
					result.IsSuccess = false;
					return result;
				}
				result.NameInstrument = NameInstrument;
				result.OpenTime = dataFile.OpenTime; // Current time as open time
				result.OpenPrice = dataFile.OpenPrice;
				result.StopLoss = StopLoss;
				result.TakeProfit = TakeProfit;
				result.Volume = Volume;
				result.ClosePrice = 0.0;
				result.TypeOrder = TypeOrder;
				result.ID = CountID;
				CountID++;
				TableActives active = {
					.NameActive = result.NameInstrument, //Require to correct this code BTC or USDT
					.Volume = Volume,
					.AccountName = AccountName,
					.OpenPrice = result.OpenPrice,
					.OpenTime = result.OpenTime,
					.TakeProfit = result.TakeProfit,
					.StopLoss = result.StopLoss,
					.TypeOrder = result.TypeOrder,
					.TimeExpiration = TimeExpiration,// Set expiration time to 1 hour from now
					.LimitOpenPrice = LimitOpenPrice,
					.IsActive = false,
					.ID = result.ID
				};
				_TableActives.push_back(active); // Add to active trades table
			}
			else
			{
#ifdef DEBUG
				CoutASSERT(MarketIsStarted == true, "Market is not started!");
#endif
				result.IsSuccess = false;
			}
		}
	}
	return result;
}

ResultOrder MarketSimulation::Market::GetRequestCloseOrder(double Volume, uint TypeOrder, const char* NameInstrument, string& AccountName)
{
	// Logic to process close order requests
	// This could involve checking if the order exists, updating balance, etc.
	ResultOrder result;
	string StrNameInstrument(NameInstrument);
	int IndexInstrument = -1;
	for (int i = 0; i < _TableActives.size(); i++)
	{
		if (_TableActives[i].NameActive == StrNameInstrument && _TableActives[i].AccountName == AccountName)
		{

			if (_TableActives[i].Volume == Volume && _TableActives[i].TypeOrder == TypeOrder)
			{
				// If the volume is equal to the active volume, close the order completely
				for (pair<const char*, vector<DataFile>>& pair : *_CurrentCandlesData) {
					if (pair.first == StrNameInstrument) {
						vector<DataFile> CurrentCandels = pair.second;
						DataFile dataFile = CurrentCandels[CurrentCandels.size()-1]; // Get the latest candle data
						result.OpenTime = _TableActives[i].OpenTime; // Current time as open time
						result.OpenPrice = _TableActives[i].OpenPrice;
						result.ClosePrice = dataFile.ClosePrice; // Assuming ClosePrice is available in DataFile
						result.StopLoss = _TableActives[i].StopLoss;
						result.TakeProfit = _TableActives[i].TakeProfit;
						result.Volume = Volume;
						result.NameInstrument = StrNameInstrument;
						result.TypeOrder = TypeOrder; // Assuming TypeOrder is defined somewhere
						result.ID = _TableActives[i].ID;
					}
				}
				_TableActives.erase(_TableActives.begin() + i); // Remove the active from the table
				result.IsSuccess = true; // Indicate that the order was successfully closed
				//result.ClosePrice; // Assuming ClosePrice is available in DataFile
				return result;
			}
			else if (_TableActives[i].Volume > Volume && _TableActives[i].TypeOrder == TypeOrder)
			{

				for (pair<const char*, vector<DataFile>>& pair : *_CurrentCandlesData) {
					if (pair.first == StrNameInstrument) {
						vector<DataFile> CurrentCandels = pair.second;
						DataFile dataFile = CurrentCandels[CurrentCandels.size()-1]; // Get the latest candle data
						result.OpenTime = _TableActives[i].OpenTime;// Current time as open time
						result.OpenPrice = _TableActives[i].OpenPrice;
						result.ClosePrice = dataFile.ClosePrice; // Assuming ClosePrice is available in DataFile
						result.StopLoss = _TableActives[i].StopLoss;
						result.TakeProfit = _TableActives[i].TakeProfit;
						result.Volume = Volume;
						result.NameInstrument = StrNameInstrument;
						result.TypeOrder = TypeOrder; // Assuming TypeOrder is defined somewhere
						result.ID = _TableActives[i].ID;
					}
				}
				_TableActives[i].Volume -= Volume;
				result.IsSuccess = true;
				// Indicate that the order was successfully closed
				result.ClosePrice;
				// Reduce the volume of the active
				return result;
			}
			else if(_TableActives[i].Volume < Volume && _TableActives[i].TypeOrder == TypeOrder)
			{
#ifdef DEBUG
				CoutASSERT(false, "Volume is greater than the active volume.");
#endif
				result.IsSuccess = false; // Indicate that the order was not successful
				return result; // Volume is greater than the active volume
			}
			IndexInstrument = i; // Store the index of the instrument
			break;
		}
	}

	result.IsSuccess = false; // Default to false if no matching active found
	return result;
}

ResultOrder MarketSimulation::Market::GetRequestCloseTakeProfitOrder(double Volume, uint TypeOrder, const char* NameInstrument, string& AccountName, double TakeProfit)
{
	// Logic to process close order requests
	// This could involve checking if the order exists, updating balance, etc.
	ResultOrder result;
	string StrNameInstrument(NameInstrument);
	int IndexInstrument = -1;
	for (int i = 0; i < _TableActives.size(); i++)
	{
		if (_TableActives[i].NameActive == StrNameInstrument && _TableActives[i].AccountName == AccountName && _TableActives[i].IsActive == true)
		{

			if (_TableActives[i].Volume == Volume && _TableActives[i].TypeOrder == TypeOrder)
			{
				// If the volume is equal to the active volume, close the order completely
				for (pair<const char*, vector<DataFile>>& pair : *_CurrentCandlesData) {
					if (pair.first == StrNameInstrument) {
						vector<DataFile> CurrentCandels = pair.second;
						DataFile dataFile = CurrentCandels[CurrentCandels.size() - 1]; // Get the latest candle data
						result.OpenTime = _TableActives[i].OpenTime; // Current time as open time
						result.OpenPrice = _TableActives[i].OpenPrice;
						result.ClosePrice = TakeProfit; 
						result.StopLoss = _TableActives[i].StopLoss;
						result.TakeProfit = _TableActives[i].TakeProfit;
						result.Volume = Volume;
						result.NameInstrument = StrNameInstrument;
						result.TypeOrder = TypeOrder; // Assuming TypeOrder is defined somewhere
						result.ID = _TableActives[i].ID;
						break;
					}
				}
				_TableActives.erase(_TableActives.begin() + i); // Remove the active from the table
				result.IsSuccess = true; // Indicate that the order was successfully closed
				//result.ClosePrice; // Assuming ClosePrice is available in DataFile
				return result;
			}
			else if (_TableActives[i].Volume > Volume && _TableActives[i].TypeOrder == TypeOrder)
			{

				for (pair<const char*, vector<DataFile>>& pair : *_CurrentCandlesData) {
					if (pair.first == StrNameInstrument) {
						vector<DataFile> CurrentCandels = pair.second;
						DataFile dataFile = CurrentCandels[CurrentCandels.size() - 1]; // Get the latest candle data
						result.OpenTime = _TableActives[i].OpenTime;// Current time as open time
						result.OpenPrice = _TableActives[i].OpenPrice;
						result.ClosePrice = TakeProfit; 
						result.StopLoss = _TableActives[i].StopLoss;
						result.TakeProfit = _TableActives[i].TakeProfit;
						result.Volume = Volume;
						result.NameInstrument = StrNameInstrument;
						result.TypeOrder = TypeOrder; // Assuming TypeOrder is defined somewhere
						result.ID = _TableActives[i].ID;
					}
				}
				_TableActives[i].Volume -= Volume;
				result.IsSuccess = true;
				// Indicate that the order was successfully closed
				result.ClosePrice;
				// Reduce the volume of the active
				return result;
			}
			else if (_TableActives[i].Volume < Volume && _TableActives[i].TypeOrder == TypeOrder)
			{
#ifdef DEBUG
				CoutASSERT(false, "Volume is greater than the active volume.");
#endif
				result.IsSuccess = false; // Indicate that the order was not successful
				return result; // Volume is greater than the active volume
			}
			IndexInstrument = i; // Store the index of the instrument
			break;
		}
	}

	result.IsSuccess = false; // Default to false if no matching active found
	return result;
}

vector<DataFile>& MarketSimulation::Market::FindCurrentCandlesData(string& NameInstrument)
{
	// Logic to find current candles data for the specified instrument
	// This could involve searching through _CurrentCandlesData stack
	vector<DataFile> emptyVector;
	if (_CurrentCandlesData == nullptr) {
#ifdef DEBUG
		CoutASSERT(false, "Current candles data is empty, cannot find data for instrument.");
#endif
		return emptyVector; // No current candles data available
	}
	for (int i = 0; i < _CurrentCandlesData->size(); i++) {
		string Str = _CurrentCandlesData[i].data()->first;
		if (Str == NameInstrument) {
			return _CurrentCandlesData[i].data()->second; // Return the found data
		}
	}
#ifdef DEBUG
	CoutASSERT(false, "No current candles data found for the specified instrument.");
#endif
	return emptyVector; // No data found for the specified instrument
}

bool MarketSimulation::Market::CheckLimitOrders()
{
	if (!MarketIsStarted)
	{
#ifdef DEBUG
		CoutASSERT(MarketIsStarted, "Market is not started!");
#endif
		return false;
	}
	if (!_TableActives.empty())
	{
		for (int i = 0; i < _TableActives.size(); i++)
		{
			pair<const char*, vector<DataFile>> *LastCurrentData = &_CurrentCandlesData->back();
			if (_TableActives[i].TypeOrder == BUY_LIMIT_ORDER || _TableActives[i].TypeOrder == SELL_LIMIT_ORDER)
			{
				if (_TableActives[i].TimeExpiration <= LastCurrentData->second.back().OpenTime) // For Cencel Orders
				{
					for (Account* Acc : _Accounts)
					{
						if (_TableActives[i].AccountName == Acc->GetNameAccount())
						{
							vector<TableCurrentOrders> *CurrentOrders = &Acc->GetCurrentOrders();
							for (int ia = 0; ia < CurrentOrders->size(); ia++)
							{
								if (_TableActives[i].NameActive == (CurrentOrders->begin()+ia)->NameInstrument
									&& _TableActives[i].TypeOrder == (CurrentOrders->begin()+ia)->TypeOrder
									&& _TableActives[i].TimeExpiration == (CurrentOrders->begin()+ia)->TimeExpiration)
								{
									_TableActives.erase(_TableActives.begin() + i);
									vector<TableHistoricalOrders>* HistoricalOrders = &Acc->GetHistoricalOrders();
									TableHistoricalOrders LineTable = {
									.OpenTime = (CurrentOrders->begin() + ia)->OpenTime,
									.OpenPrice = 0.0,
									.ClosePrice = 0.0,
									.StopLoss = (CurrentOrders->begin() + ia)->StopLoss,
									.TakeProfit = (CurrentOrders->begin() + ia)->TakeProfit,
									.Volume = (CurrentOrders->begin() + ia)->Volume,
									.TypeOrder = (CurrentOrders->begin() + ia)->TypeOrder,
									.TimeExpiration = (CurrentOrders->begin() + ia)->TimeExpiration
									};////
									HistoricalOrders->push_back(LineTable);
									CurrentOrders->erase((CurrentOrders->begin() + ia));
									cout << "Order ID: " << (CurrentOrders->begin() + ia)->ID 
										<< ", Instrument: " << (CurrentOrders->begin() + ia)->NameInstrument
										<< ", Volume: " << (CurrentOrders->begin() + ia)->Volume 
										<< ", AccountName: " << Acc->GetNameAccount() << endl;
									cout << "Limit Order was cenceled!" << endl;
									i--;
									ia--;
								}


							}
						}
					}
					
				}
				else if(_TableActives[i].IsActive == false)
				if((_TableActives[i].TypeOrder == BUY_LIMIT_ORDER 
					&& _TableActives[i].LimitOpenPrice <= LastCurrentData->second.back().HighPrice 
					&& _TableActives[i].LimitOpenPrice <= LastCurrentData->second.back().LowPrice) || 
					(_TableActives[i].TypeOrder == SELL_LIMIT_ORDER
						&& _TableActives[i].LimitOpenPrice >= LastCurrentData->second.back().HighPrice
						&& _TableActives[i].LimitOpenPrice >= LastCurrentData->second.back().LowPrice)
					&& _TableActives[i].NameActive == LastCurrentData->first)
				{ 
					_TableActives[i].OpenTime = LastCurrentData->second.back().OpenTime;
					_TableActives[i].OpenPrice = _TableActives[i].LimitOpenPrice;
					_TableActives[i].IsActive = true;
					for (Account* Acc : _Accounts)
					{
						if(Acc->GetNameAccount() == _TableActives[i].AccountName)
						{ 
							vector<TableCurrentOrders> *CurrentOrders = &Acc->GetCurrentOrders();
							for (auto it = CurrentOrders->begin(); it != CurrentOrders->end(); it++)
							{
								if (it->NameInstrument == string(LastCurrentData->first) 
									&& it->LimitOpenPrice == _TableActives[i].LimitOpenPrice
									&& it->Volume == _TableActives[i].Volume)
								{
									it->OpenTime = LastCurrentData->second.back().OpenTime;
									it->OpenPrice = _TableActives[i].LimitOpenPrice;
									cout << "Order ID: " << it->ID
										<< ", Instrument: " << it->NameInstrument
										<< ", Volume: " << it->Volume
										<< ", AccountName: " << Acc->GetNameAccount() << endl;
									cout << "Limit Order was opened!" << endl;
								}
							}
						}
					}
				}
			}
		}
	}
	return true;
}

bool MarketSimulation::Market::CheckTakeProfits()
{
	if (MarketIsStarted)
	{
		ResultOrder Result;
		Result.IsSuccess = false;
		string AccountName;
		for (auto itCandle = _CurrentCandlesData->begin(); itCandle != _CurrentCandlesData->end(); itCandle++)
			for (int i = 0; i < _TableActives.size(); i++)
			{
				if (_TableActives[i].NameActive == string(itCandle->first) && _TableActives[i].IsActive == true)
				{
					if (_TableActives[i].TypeOrder == BUY_ORDER || _TableActives[i].TypeOrder == BUY_LIMIT_ORDER)
					{
						vector<DataFile>* CurrentCandles = &itCandle->second;
						if (CurrentCandles->back().LowPrice <= _TableActives[i].TakeProfit || CurrentCandles->back().HighPrice <= _TableActives[i].TakeProfit)
						{
							AccountName = _TableActives[i].AccountName;
							Result = GetRequestCloseTakeProfitOrder(_TableActives[i].Volume, _TableActives[i].TypeOrder, _TableActives[i].NameActive.c_str(), _TableActives[i].AccountName, _TableActives[i].TakeProfit);
							i--;
							if (Result.IsSuccess)
							{
								for (Account* Acc : _Accounts)
								{
									if (Acc->GetNameAccount() == AccountName)
									{
										vector<TableHistoricalOrders>* HistoricalOrders = &Acc->GetHistoricalOrders();
										TableHistoricalOrders LineTable = {
										.OpenTime = Result.OpenTime,
										.OpenPrice = Result.OpenPrice,
										.ClosePrice = Result.ClosePrice,
										.StopLoss = Result.StopLoss,
										.TakeProfit = Result.TakeProfit,
										.Volume = Result.Volume,
										.TypeOrder = Result.TypeOrder,
										.TimeExpiration = 0,
										.ID = Result.ID
										};
										vector<TableCurrentOrders>* AccCurrentOrders = &Acc->GetCurrentOrders();
										for (auto itOrder = AccCurrentOrders->begin(); itOrder != AccCurrentOrders->end(); itOrder++)
											if (itOrder->ID == LineTable.ID)
											{
												AccCurrentOrders->erase(itOrder);
												break;
											}
										double AddMoney = 0;
										if (Result.TypeOrder == BUY_ORDER || Result.TypeOrder == BUY_LIMIT_ORDER)
										{
											AddMoney = Result.ClosePrice * Result.Volume;
										}
										else if (Result.TypeOrder == SELL_ORDER || Result.TypeOrder == SELL_LIMIT_ORDER)
										{
											AddMoney = (Result.OpenPrice * Result.Volume - Result.ClosePrice * Result.Volume) + Result.ClosePrice * Result.Volume; // Adjust for SELL order
										}
										Acc->SetBalance(AddMoney); // Add the closed order amount to the balance
										cout << "Order ID: " << Result.ID
											<< ", Instrument: " << Result.NameInstrument
											<< ", Volume: " << Result.Volume
											<< ", AccountName: " << Acc->GetNameAccount() << endl;
										cout << "Order was closed with TakeProfit!" << endl;
										HistoricalOrders->push_back(LineTable);
									}
								}
							}
						}
					}
					else if (_TableActives[i].TypeOrder == SELL_ORDER || _TableActives[i].TypeOrder == SELL_LIMIT_ORDER)
					{
						vector<DataFile>* CurrentCandles = &itCandle->second;
						if (CurrentCandles->back().LowPrice >= _TableActives[i].TakeProfit || CurrentCandles->back().HighPrice >= _TableActives[i].TakeProfit)
						{
							AccountName = _TableActives[i].AccountName;
							Result = GetRequestCloseTakeProfitOrder(_TableActives[i].Volume, _TableActives[i].TypeOrder, _TableActives[i].NameActive.c_str(), _TableActives[i].AccountName, _TableActives[i].TakeProfit);
							i--;
							if (Result.IsSuccess)
							{
								for (Account* Acc : _Accounts)
								{
									if (Acc->GetNameAccount() == AccountName)
									{
										vector<TableHistoricalOrders>* HistoricalOrders = &Acc->GetHistoricalOrders();
										TableHistoricalOrders LineTable = {
										.OpenTime = Result.OpenTime,
										.OpenPrice = Result.OpenPrice,
										.ClosePrice = Result.ClosePrice,
										.StopLoss = Result.StopLoss,
										.TakeProfit = Result.TakeProfit,
										.Volume = Result.Volume,
										.TypeOrder = Result.TypeOrder,
										.TimeExpiration = 0,
										.ID = Result.ID
										};
										vector<TableCurrentOrders>* AccCurrentOrders = &Acc->GetCurrentOrders();
										for (auto itOrder = AccCurrentOrders->begin(); itOrder != AccCurrentOrders->end(); itOrder++)
											if (itOrder->ID == LineTable.ID)
											{
												AccCurrentOrders->erase(itOrder);
												break;
											}
										double AddMoney = 0;
										if (Result.TypeOrder == BUY_ORDER || Result.TypeOrder == BUY_LIMIT_ORDER)
										{
											AddMoney = Result.ClosePrice * Result.Volume;
										}
										else if (Result.TypeOrder == SELL_ORDER || Result.TypeOrder == SELL_LIMIT_ORDER)
										{
											AddMoney = (Result.OpenPrice * Result.Volume - Result.ClosePrice * Result.Volume) + Result.ClosePrice * Result.Volume; // Adjust for SELL order
										}
										Acc->SetBalance(AddMoney); // Add the closed order amount to the balance
										cout << "Order ID: " << Result.ID
											<< ", Instrument: " << Result.NameInstrument
											<< ", Volume: " << Result.Volume
											<< ", AccountName: " << Acc->GetNameAccount() << endl;
										cout << "Order was closed with TakeProfit!" << endl;
										HistoricalOrders->push_back(LineTable);
									}
								}
							}
						}
					}
					else break;
				}
			}
		

		
		return false;
	}
}

bool MarketSimulation::Market::CheckStopLoss()
{
	bool TrueCheck = false;
	if (MarketIsStarted)
	{
		ResultOrder Result;
		Result.IsSuccess = false;
		string AccountName;
		for (auto itCandle = _CurrentCandlesData->begin(); itCandle != _CurrentCandlesData->end(); itCandle++)
			for (int i = 0; i < _TableActives.size(); i++)
			{
				if (_TableActives[i].NameActive == string(itCandle->first) && _TableActives[i].IsActive == true)
				{
					if (_TableActives[i].TypeOrder == BUY_ORDER || _TableActives[i].TypeOrder == BUY_LIMIT_ORDER)
					{
						vector<DataFile>* CurrentCandles = &itCandle->second;
						if (CurrentCandles->back().LowPrice >= _TableActives[i].StopLoss || CurrentCandles->back().HighPrice >= _TableActives[i].StopLoss)
						{
							AccountName = _TableActives[i].AccountName;
							Result = GetRequestCloseTakeProfitOrder(_TableActives[i].Volume, _TableActives[i].TypeOrder, _TableActives[i].NameActive.c_str(), _TableActives[i].AccountName, _TableActives[i].StopLoss);
							i--;
							if (Result.IsSuccess)
							{
								for (Account* Acc : _Accounts)
								{
									if (Acc->GetNameAccount() == AccountName)
									{
										vector<TableHistoricalOrders>* HistoricalOrders = &Acc->GetHistoricalOrders();
										TableHistoricalOrders LineTable = {
										.OpenTime = Result.OpenTime,
										.OpenPrice = Result.OpenPrice,
										.ClosePrice = Result.ClosePrice,
										.StopLoss = Result.StopLoss,
										.TakeProfit = 0,
										.Volume = Result.Volume,
										.TypeOrder = Result.TypeOrder,
										.TimeExpiration = 0,
										.ID = Result.ID
										};
										vector<TableCurrentOrders>* AccCurrentOrders = &Acc->GetCurrentOrders();
										for (auto itOrder = AccCurrentOrders->begin(); itOrder != AccCurrentOrders->end(); itOrder++)
											if (itOrder->ID == LineTable.ID)
											{
												AccCurrentOrders->erase(itOrder);
												break;
											}
										double AddMoney = 0;
										if (Result.TypeOrder == BUY_ORDER || Result.TypeOrder == BUY_LIMIT_ORDER)
										{
											AddMoney = (Result.OpenPrice * Result.Volume - Result.ClosePrice * Result.Volume) + Result.ClosePrice * Result.Volume; // Adjust for SELL order
										}
										else if (Result.TypeOrder == SELL_ORDER || Result.TypeOrder == SELL_LIMIT_ORDER)
										{
											AddMoney = Result.ClosePrice * Result.Volume;
										}
										Acc->TakeAwayBalance(AddMoney); // Add the closed order amount to the balance
										cout << "Order ID: " << Result.ID
											<< ", Instrument: " << Result.NameInstrument
											<< ", Volume: " << Result.Volume
											<< ", AccountName: " << Acc->GetNameAccount() << endl;
										cout << "Order was closed with StopLoss!" << endl;
										HistoricalOrders->push_back(LineTable);

									}
								}
							}
						}
					}
					else if (_TableActives[i].TypeOrder == SELL_ORDER || _TableActives[i].TypeOrder == SELL_LIMIT_ORDER)
					{
						vector<DataFile>* CurrentCandles = &itCandle->second;
						if (CurrentCandles->back().LowPrice <= _TableActives[i].StopLoss || CurrentCandles->back().HighPrice <= _TableActives[i].StopLoss)
						{
							AccountName = _TableActives[i].AccountName;
							Result = GetRequestCloseTakeProfitOrder(_TableActives[i].Volume, _TableActives[i].TypeOrder, _TableActives[i].NameActive.c_str(), _TableActives[i].AccountName, _TableActives[i].StopLoss);
							i--;
							if (Result.IsSuccess)
							{
								for (Account* Acc : _Accounts)
								{
									if (Acc->GetNameAccount() == AccountName)
									{
										vector<TableHistoricalOrders>* HistoricalOrders = &Acc->GetHistoricalOrders();
										TableHistoricalOrders LineTable = {
										.OpenTime = Result.OpenTime,
										.OpenPrice = Result.OpenPrice,
										.ClosePrice = Result.ClosePrice,
										.StopLoss = Result.StopLoss,
										.TakeProfit = 0,
										.Volume = Result.Volume,
										.TypeOrder = Result.TypeOrder,
										.TimeExpiration = 0,
										.ID = Result.ID
										};
										vector<TableCurrentOrders>* AccCurrentOrders = &Acc->GetCurrentOrders();
										for (auto itOrder = AccCurrentOrders->begin(); itOrder != AccCurrentOrders->end(); itOrder++)
											if (itOrder->ID == LineTable.ID)
											{
												AccCurrentOrders->erase(itOrder);
												break;
											}
										double AddMoney = 0;
										if (Result.TypeOrder == BUY_ORDER || Result.TypeOrder == BUY_LIMIT_ORDER)
										{
											AddMoney = (Result.OpenPrice * Result.Volume - Result.ClosePrice * Result.Volume) + Result.ClosePrice * Result.Volume; // Adjust for SELL order
										}
										else if (Result.TypeOrder == SELL_ORDER || Result.TypeOrder == SELL_LIMIT_ORDER)
										{
											AddMoney = Result.ClosePrice * Result.Volume;
										}
										Acc->TakeAwayBalance(AddMoney); // Add the closed order amount to the balance
										cout << "Order ID: " << Result.ID
											<< ", Instrument: " << Result.NameInstrument
											<< ", Volume: " << Result.Volume
											<< ", AccountName: " << Acc->GetNameAccount() << endl;
										cout << "Order was closed with StopLoss!" << endl;
										HistoricalOrders->push_back(LineTable);

									}
								}
							}
						}
					}
					else break;
				}
			}
		
	}
	return false;
}


std::chrono::time_point<std::chrono::system_clock> MarketSimulation::GetTimeDate(long long Time)
{
	// Convert time in milliseconds to a human-readable format  
	std::chrono::milliseconds ms(Time);
	std::chrono::time_point<std::chrono::system_clock> time_point = std::chrono::time_point<std::chrono::system_clock>(ms);

	// Get time in time_t format  
	//std::time_t time = std::chrono::system_clock::to_time_t(time_point);

	// Convert time_t to tm structure for formatting  
	/*std::tm timeinfo;
#ifdef _WIN32  
	localtime_s(&timeinfo, &time);
#else  
	localtime_r(&time, &timeinfo);
#endif  
	*/
	// Format date and time  
	//char buffer[80];
	//cout << "Time: " << time_point << endl;
	//std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
	return time_point;
}
void MarketSimulation::RunSimulation(Market& Market, ExpertAdviser& Adviser)
{
	
	Market.StartMarket();
	CurrentCandlesIndexes *CurrentCandlesData = new CurrentCandlesIndexes;
	Market.SetPointerForCurrentCandlesData(CurrentCandlesData);
	Market.PrepareCurrentCandlesData();
	auto List = Market.GetListInstruments();
	auto Chart = Adviser.GetChart();
	Adviser.Init();
	for (int i = 0; i < CurrentCandlesData->size(); i++)
	{
		if (strcmp(Chart.GetNameChart(), CurrentCandlesData->data()->first))
		{
			Chart.SetPointerCurrentCandlesData(&CurrentCandlesData->data()->second);
			break;
		}
	}
	vector<int> Indexes;
	int i = 0;
	for (const auto& instrument : List) {
		auto Data = instrument.GetCandlesData();
		Indexes.push_back(Data.size()); // Store the size of each instrument's candles data
	}
	int HighestIndex = -1;
	int Size = 0;
	for (i = 0; i < Indexes.size(); i++)
	{
		if (Indexes[i] > Size)
		{
			HighestIndex = i;
			Size = Indexes[i];
		}
	}
	if (HighestIndex == -1)
	{
		CoutASSERT(false, "No valid instrument found with candles data.");
		return; // No valid instrument found
	}
	auto DataOfInstrument = List[HighestIndex].GetCandlesData();
	i = 0;
	cout << "LoadStrategy..." << endl;
	while (DataOfInstrument.size() != i)
	{
		for (int it = 0; it < List.size(); it++)
		{
			auto REF = CurrentCandlesData[it].data();
			const char* CStr = REF->first;
			if (List[it].GetNameInstrument() == CStr && List[it].GetCandlesData().size() > i)
			{
				CurrentCandlesData->data()->second.push_back(DataOfInstrument[i]); // Push the first candle data
			}
		}
		i++;
		Market.CheckLimitOrders();
		Market.CheckTakeProfits();
		Market.CheckStopLoss();
		Adviser.LoadStrategy();
	}
	Market.StopMarket();

	
}


TableCurrentOrders MarketSimulation::Account::GetCurrentOrder(uint ID)
{
	TableCurrentOrders EmptyOrder = {
	 .OpenTime = 0,
	 .OpenPrice = 0.0,
	 .StopLoss = 0.0,
	 .TakeProfit = 0.0,
	.Volume = 0.0,
	.TypeOrder = 0, // "Buy" or "Sell"
	.LimitOpenPrice = 0.0, // Price at which the order is opened
	.TimeExpiration = 0, // Time when the order expires, if applicable
	.ID = 0
	};
	for (auto iter = _CurrentOrders.begin(); iter != _CurrentOrders.end(); iter++)
		if (iter->ID == ID)
		{
			return *iter;
		}
	return EmptyOrder;
}




vector<double> MarketSimulation::Chart::GetData(uint IndicatorType)
{
	auto IData = GetIndicatorsData();
	int Index = 0;
	for (auto iter = IData.begin(); iter != IData.end(); iter++)
	{
		int IndexIter = Index;
		if (_Indicators[IndexIter].first == IndicatorType)
		{

			//cout << "View NATR Indicator Data: " << endl;
			return *iter;
		}
		else if (_Indicators[IndexIter].first == IndicatorType)
		{
			//cout << "View EMA Indicator Data: " << endl;
			return *iter;
		}
		Index++;
	}
	vector<double> emptyVector; // Return an empty vector if no data found
	return emptyVector;
}

void MarketSimulation::Chart::Refresh(void)
{
	if (!_Indicators.empty())
	{
		if (!_DataIndicators.empty()) {
			_DataIndicators.clear(); // Clear previous indicators data
		}
			
		for (auto it = _Indicators.begin(); it < _Indicators.end(); it++)// Initialize if not already done
			{
				if (it->first == INATR)
				{
					vector<OHLC> CandlesData;
					if(_CurrentCandlesData->size() <= it->second.ViewRange)
					{
						for (auto iter = _CurrentCandlesData->begin(); iter != _CurrentCandlesData->end(); iter++) {
							CandlesData.push_back({ iter->HighPrice, iter->LowPrice, iter->ClosePrice });
						}

					}
					else for (auto iter = _CurrentCandlesData->begin() + (_CurrentCandlesData->size() - it->second.ViewRange); iter != _CurrentCandlesData->end(); iter++) {
						CandlesData.push_back({ iter->HighPrice, iter->LowPrice, iter->ClosePrice });
					}
					auto ResultData = calculateNATR(CandlesData, it->second.Period);
					_DataIndicators.push_back(ResultData); // Store NATR results);
				}
				else if (it->first == IEMA || it->first == ISMA)
				{
					
					int Index;
					if (_CurrentCandlesData->size() <= it->second.ViewRange)
						Index = 0;
					else Index = _CurrentCandlesData->size() - it->second.ViewRange;
					vector<DataFile> CandlesData;
					CandlesData.insert(CandlesData.begin(), _CurrentCandlesData->begin() + Index, _CurrentCandlesData->end());
					vector<double> IndicatorsData;
					for (auto iter = _CurrentCandlesData->begin() + Index; iter != _CurrentCandlesData->end(); iter++) {
						switch (it->second.OptionPrice)
						{
						case OPEN_PRICE:
							IndicatorsData.push_back({ iter->OpenPrice });
							continue;
						case CLOSE_PRICE:
							IndicatorsData.push_back({ iter->ClosePrice });
							continue;
						case HIGH_PRICE:
							IndicatorsData.push_back({ iter->HighPrice });
							continue;
						case LOW_PRICE:
							IndicatorsData.push_back({ iter->LowPrice });
							continue;
						default:
							//CoutASSERT(false, "Invalid option price for EMA indicator.");
							throw std::runtime_error("Invalid option price for EMA indicator.");
							return;
						}
					}
					if (it->first == IEMA)
					{
						//vector<double> ResultData = calculateEMA(IndicatorsData, it->second.Period);
						vector<double> ResultData = calculateEMA(IndicatorsData, it->second.Period);
						_DataIndicators.push_back(ResultData); // Store EMA results
					}
					else if (it->first == ISMA)
					{
						//cout << "View SMA Indicator Data: " << endl;
						//vector<double> ResultData = calculateEMA(IndicatorsData, it->second.Period);
						vector<double> ResultData = TA_SMA(IndicatorsData, it->second.Period);
						_DataIndicators.push_back(ResultData); // Store NATR results)
					}
					
				}

			}
		for (const auto& indicator : _Indicators) {

			if (indicator.first == INATR) {
				cout << "Refreshing NATR indicator with period: " << indicator.second.Period << endl;
			}
			else if (indicator.first == IEMA) {
				cout << "Refreshing EMA indicator with period: " << indicator.second.Period << endl;
			}
		}
	}
	cout << "Chart refreshed." << endl;
}


