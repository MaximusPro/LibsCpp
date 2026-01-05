// MarketSimulation.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "Defines.h"
#include <string>
#include "Indicators.h"
#include <chrono>
#include <ctime>
#include <iomanip>


namespace MarketSimulation {
	typedef vector<pair<const char*, stack<DataFile>>> CurrentCandlesStack;
	typedef vector<pair<const char*, vector<DataFile>>> CurrentCandlesIndexes; // Indexes of current candles data for each instrument

#define SELL_ORDER 0
#define BUY_ORDER 1
#define SELL_LIMIT_ORDER 2
#define BUY_LIMIT_ORDER 3
#define INATR 0
#define IEMA 1
#define ISMA 2
#define OPEN_PRICE 45
#define CLOSE_PRICE 46
#define HIGH_PRICE 47
#define LOW_PRICE 48
#define MIN_PRICE 49
#define MAX_PRICE 50
#define SECOND 1000
#define MINUT 60000
#define HOUR 360000
	
	

	class Instrument {
	private:
		string _NameInstrument;
		vector<DataFile> *_CandlesData;
	public:
		//Instrument(void) {}
		Instrument(const std::string& name) : _NameInstrument(name) {
			_CandlesData = nullptr;
		}
		Instrument(const char* name) : _NameInstrument(name) {
			_CandlesData = nullptr;
		}
		Instrument(const char* Name, vector<DataFile>& Data) {
			_NameInstrument = Name;
			_CandlesData = new vector<DataFile>(Data);
			if (_CandlesData == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Failed to allocate memory for candles data.");
#endif
				
				throw std::runtime_error("Failed to allocate memory for candles data.");
			}
		}
		bool LoadData(vector<DataFile>& Data) {
			return true;
		}
		const string& GetNameInstrument() const { return _NameInstrument; }
		const vector<DataFile>& GetCandlesData() const {
			if (_CandlesData == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Candles data is null, cannot get candles data.");
#endif
				throw std::runtime_error("Candles data is null.");
			}
			return *_CandlesData; 
		}
		~Instrument(void) {}
		
	};
	typedef struct TableHistoricalOrders
	{
		long long OpenTime;
		double OpenPrice;
		double ClosePrice;
		double StopLoss;
		double TakeProfit;
		double Volume;
		uint TypeOrder; // "Buy" or "Sell"
		long long TimeExpiration;
		uint ID;
	} TableHistoricalOrders;
	typedef struct TableCurrentOrders
	{
		string NameInstrument; // Name of the instrument for which the order is placed
		long long OpenTime;
		double OpenPrice;
		double StopLoss;
		double TakeProfit;
		double Volume;
		uint TypeOrder; // "Buy" or "Sell"
		double LimitOpenPrice; // Price at which the order is opened
		long long TimeExpiration; // Time when the order expires, if applicable
		uint ID;
	} TabelCurrentOrders;
	typedef struct ResultOrder
	{
		string NameInstrument; // Name of the instrument for which the order is placed
		long long OpenTime;
		double OpenPrice;
		double ClosePrice;
		double StopLoss;
		double TakeProfit;
		double Volume;
		uint TypeOrder; // "Buy" or "Sell"
		bool IsSuccess;// true if order was successful, false otherwise
		uint ID;
	} ResultOrder;
	typedef struct TableActives
	{
		string NameActive;
		double Volume;
		string AccountName;
		double OpenPrice; // Price at which the active was opened
		long long OpenTime; // Time when the active was opened
		double TakeProfit; // Take profit level for the active
		double StopLoss;
		uint TypeOrder;
		long long TimeExpiration;
		double LimitOpenPrice; // Time when the active expires, if applicable
		bool IsActive;
		uint ID;
	} TableActives;

	typedef struct IParameters
	{
		int Period; // Period for the indicator
		uint ViewRange; // View range for the indicator, e.g., number of candles to display
		uint OptionPrice; // Additional options for the indicator, if needed
	} IParameters;
	typedef vector<pair<int, IParameters>> VecIndicators;
	using std::chrono::time_point;

	std::chrono::time_point<std::chrono::system_clock> GetTimeDate(long long Time);
	class Account;
	class Market {
	private:
		uint CountID = 1;
		vector<Instrument> _Instruments;
		CurrentCandlesIndexes* _CurrentCandlesData; // Stack of current candles data for each instrument
		vector<TableActives> _TableActives;
		vector<pair<const char*, int>> _TableActivesIndex;
		vector<Account*> _Accounts;
		// Index of active instruments
		bool MarketIsStarted;
	public:
		Market(void) {
			_CurrentCandlesData = nullptr;
		}
		Market(const vector<Instrument>& instruments) : _Instruments(instruments) {
			_CurrentCandlesData = nullptr;
		}
		Market(const Instrument& Instrument) {
			_CurrentCandlesData = nullptr;
			_Instruments.push_back(Instrument);
		}
		bool AddInstrument(const Instrument& instrument) {
			_Instruments.push_back(instrument);
			return true;
		}
		void AddAccount(Account* Acc)
		{
			_Accounts.push_back(Acc);
		}
		//const vector<Instrument>& GetInstruments() const { return _Instruments; }
		bool StartMarket(){
			// Logic to start the market simulation
			// This could involve initializing instruments, setting up accounts, etc.
			MarketIsStarted = true;
			return true;
		}
		ResultOrder GetRequestOpenOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, const char* NameInstrument, string& AccountName);
		ResultOrder GetRequestOpenLimitOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, const char* NameInstrument, string& AccountName, double LimitOpenPrice, long long TimeExpiration);
		ResultOrder GetRequestCloseOrder(double Volume, uint TypeOrder, const char* NameInstrument, string& AccountName);
		ResultOrder GetRequestCloseTakeProfitOrder(double Volume, uint TypeOrder, const char* NameInstrument, string& AccountName, double TakeProfit);
		bool StopMarket() {
			// Logic to stop the market simulation
			// This could involve finalizing trades, closing accounts, etc.
			MarketIsStarted = false;
			return true;
		}
		vector<Instrument>GetListInstruments() const {
			// Logic to get the list of instruments
			// This could involve returning the _Instruments vector
			return _Instruments;
		}
		void SetPointerForCurrentCandlesData(CurrentCandlesIndexes* CurrentCandlesData) { //delete this code
			_CurrentCandlesData = CurrentCandlesData;
		}
		void SetPointerCurrentCandlesIndexes(vector<pair<const char*, int>>* CurrentCandlesIndexes) {
			_TableActivesIndex = *CurrentCandlesIndexes;
		}
		void PrepareCurrentCandlesData() {
			// Logic to prepare current candles data
			// This could involve populating _CurrentCandlesData with data from instruments
			for (const auto& instrument : _Instruments) {
				vector<DataFile> currentCandles; 
				_CurrentCandlesData->emplace_back(instrument.GetNameInstrument().c_str(), currentCandles);
			}
		}
		vector<DataFile>& FindCurrentCandlesData(string& NameInstrument);
		bool CheckLimitOrders();
		bool CheckTakeProfits();
		bool CheckStopLoss();
		~Market(void) {}

	};
	
	class Account {
	private:
		string _NameAccount;
		double _Balance;
		
		//double _Leverage;
		vector<Instrument>* _Instruments;
		vector<TableHistoricalOrders> _HistoricalOrders;
		vector<TableCurrentOrders> _CurrentOrders;
	public:
		Account(const std::string& name, double balance)
			: _NameAccount(name), _Balance(balance) {
			_Instruments = nullptr;
		}
		Account(const char* name, double balance)
			: _NameAccount(name), _Balance(balance) {
			_Instruments = nullptr;
		}

		bool AddInstrument(const Instrument& instrument) {
			_Instruments->push_back(instrument);
			return true;
		}
		const string& GetNameAccount() const { return _NameAccount; }
		uint OpenOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, Market& Market, const char* NameInstrument);
		uint OpenLimitOrder(double Volume, uint TypeOrder, double TakeProfit, double StopLoss, Market& Market, const char* NameInstrument, double LimitOpenPrice, long long TimeExpiration);
		bool CloseOrder(double Volume, uint TypeOrder, const char* NameInstrument, Market& Market);
		double GetBalance() const { return _Balance; }
		void SetBalance(double balance) { _Balance += balance; }
		void TakeAwayBalance(double balance) {
			if (_Balance >= balance) {
				_Balance -= balance;
			}
			else {
				_Balance = 0.0; // Set balance to zero if not enough funds
			}
		}
		bool IsEnoughMoney(double Money)
		{
			if (_Balance >= Money)
				return true;
			return false;
		}
		const vector<Instrument>& GetInstruments() const { return *_Instruments; }
		TableCurrentOrders GetCurrentOrder(uint OrderID) const {
			// Logic to get current order by ID
			// This could involve searching through _CurrentOrders vector
			TableCurrentOrders order;
			if (OrderID >= _CurrentOrders.size()) {
				throw std::out_of_range("Order ID is out of range.");
			}
			else
			 order = _CurrentOrders[OrderID];
			return order;
		}
		vector<TableCurrentOrders> &GetCurrentOrders() {
			// Logic to get all current orders
			// This could involve returning the _CurrentOrders vector
			return _CurrentOrders;
		}
		TableCurrentOrders GetCurrentOrder(uint ID);
		vector<TableHistoricalOrders>& GetHistoricalOrders() {
			return _HistoricalOrders;
		}
		bool ExistOrder(uint ID)
		{
			for (auto iter = _CurrentOrders.begin(); iter != _CurrentOrders.end(); iter++)
				if (iter->ID == ID)
					return true;
			return false;
		}
		~Account(void) {}
	};

	class Chart {
	private:
		string _NameChart;
		Instrument* _Inst;
		vector<DataFile>* _CurrentCandlesData; // Pointer to candles data
		VecIndicators _Indicators;
		vector<vector<double>> _DataIndicators; // List of indicators added to the chart
	public:
		Chart(const std::string& name) : _NameChart(name) {
			_Inst = nullptr;
			_CurrentCandlesData = nullptr;
		}
		Chart(const char* name) : _NameChart(name) {
			_Inst = nullptr;
			_CurrentCandlesData = nullptr;
		}
		Chart(const char* name, Instrument& instrument) : _NameChart(name) {
			_Inst = &instrument;
			_CurrentCandlesData = nullptr;
		}
		vector<DataFile> GetCandlesData() { 
			if (_Inst == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Instrument pointer is null, cannot get candles data.");
#endif
				throw std::runtime_error("Instrument pointer is null.");
				//return nullptr;
			}
			cout << "_Inst->GetCandlesData().size(): " << _Inst->GetCandlesData().size() << endl;
			return _Inst->GetCandlesData(); }
		Instrument& GetInstrument()
		{
			if (_Inst == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Instrument pointer is null, cannot get instrument.");
#endif
				throw std::runtime_error("Instrument pointer is null.");
			}
			return *_Inst;
		}
		vector<DataFile> GetCurrentCandlesData() {
			if (_CurrentCandlesData == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Current candles data pointer is null, cannot get current candles data.");
#endif
				throw std::runtime_error("Current candles data pointer is null.");
			}
			return *_CurrentCandlesData;
		}
		void SetPointerCurrentCandlesData(vector<DataFile>* CurrentCandlesData) {
			_CurrentCandlesData = CurrentCandlesData;
		}
		bool IsPointerCurrentCandlesData() {
			if (_CurrentCandlesData == nullptr) {
				return false; // Pointer is null
			}
			return true; // Pointer is valid
		}
		const char* GetNameChart() const { return _NameChart.c_str(); }
		void AddINATR(int period, uint ViewRange = 40) {
			// Logic to add NATR indicator to the chart
			// This could involve calculating NATR values and storing them in _Indicators vector
			auto Indicator = make_pair(INATR, IParameters{ period, ViewRange, });
			_Indicators.push_back(Indicator);
			cout << "Added NATR indicator with period: " << period << endl;
		}
		void AddIEMA(int period, uint ViewRange = 40, uint OptionPrice=CLOSE_PRICE) {
			// Logic to add EMA indicator to the chart
			// This could involve calculating EMA values and storing them in _Indicators vector
			auto Indicator = make_pair(IEMA, IParameters{ period, ViewRange, OptionPrice});
			_Indicators.push_back(Indicator);
			cout << "Added EMA indicator with period: " << period << endl;
		}
		void AddISMA(int period, uint ViewRange = 40, uint OptionPrice = CLOSE_PRICE)
		{
			auto Indicator = make_pair(ISMA, IParameters{ period, ViewRange, OptionPrice });
			_Indicators.push_back(Indicator);
			cout << "Added SEMA indicator with period: " << period << endl;
		}
		VecIndicators& GetIndicatorsParameters()
		{
			return _Indicators;
		}
		vector<vector<double>>& GetIndicatorsData()
		{
			return _DataIndicators;
		}
		vector<double> GetData(uint IndicatorType);
		
		void Refresh(void);
		~Chart(void) {}
	};

	class ExpertAdviser {
	private:
		string _NameAdviser;
		Account* _Account;
		Market* _Market;
		Chart* _Chart;
	public:
		//ExpertAdviser(const char* NameAdviser) { _NameAdviser = NameAdviser; }
		ExpertAdviser(const char* adviserName, Account& accountPtr, Market& Market, Chart& Chart)
			: _NameAdviser(adviserName), _Account(&accountPtr), _Market(&Market), _Chart(&Chart){
		}
		//ExpertAdviser(const std::string& NameAdviser) { _NameAdviser = NameAdviser; }
		Chart& GetChart() {
			if (_Chart == nullptr) {
#ifdef DEBUG
				CoutASSERT(false, "Chart pointer is null, cannot get chart.");
#endif
				throw std::runtime_error("Chart pointer is null.");
			}
			return *_Chart;
		}
		void Init() {
			// Logic to initialize the expert adviser
			// This could involve setting up indicators, preparing data, etc.
			cout << "Initializing Expert Adviser: " << _NameAdviser << endl;
		}
		void LoadStrategy() {}
	};

	void RunSimulation(Market& Market, ExpertAdviser& Adviser);
	

	
    

}
// TODO: Reference additional headers your program requires here.
