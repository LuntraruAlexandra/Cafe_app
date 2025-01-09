#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <ctime>
#include <queue>
#include <memory>
#include <utility>
#include <iomanip>
#include <cstdio>
#define RESET   "\033[0m"
#define YELLOW  "\033[1;95m"
#define GREEN   "\033[1;32m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
using namespace std;

string lang = "en";
string in_folder="";
string out_folder="";

string translate_to_romanian(string text){
    ifstream fin("translate.txt");

    string line;
    while(getline(fin,line)){
        stringstream ss(line);
        string word, translate;
        getline(ss,word,'=');
        getline(ss,translate,';');
        if(word == text)
            return translate;
    }
    fin.close();
    return text; //returns the same text if we can't have a translation
    
}
string translate_to_english(string text){
    ifstream fin("translate.txt");

    string line;
    while(getline(fin,line)){
        stringstream ss(line);
        string word, translate;
        getline(ss,word,'=');
        getline(ss,translate,';');
        if(translate == text)
            return word;
    }
    fin.close();
    return text;
    
}
void addTranslate(const string& text, const string& translate) {
    string filename = "translate.txt";
    ofstream fout(filename, ios::app);
    fout << text << "=" << translate << ";";
    fout<<"\n";
    fout.close();
}
//Singleton Design Pattern because we only need one instance to create the folders
class CreateFolders { //class that creates the 5 folders that correspond to every city
private:
    static CreateFolders* instance;
    static once_flag flag;

    CreateFolders(const CreateFolders&) = delete;
    CreateFolders& operator=(const CreateFolders&) = delete;

    CreateFolders() {} // private constructor
    ~CreateFolders() {} //destructor

public:
    static CreateFolders* getInstance();
    void createDirectories() {
    vector<string> cities = {"București", "Cluj-Napoca", "Timișoara", "Iași", "Brașov"};

    for (const auto& city : cities) {
        try {
            if (filesystem::create_directory(in_folder +"/" + city)) {
                cout << "Directory created successfully: " << city << endl;
            } else {
                //cout << "Directory already exists: " << city << endl;
            }
        } catch (const filesystem::filesystem_error& e) {
            cerr << "Error creating directory (" << city << "): " << e.what() << endl;
        }
    }
}
       void createCityFiles() {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        string header_report = to_string(now->tm_mday)+"/"+to_string(now->tm_mon+1)+"/"+to_string(now->tm_year+1900)+"\n";
        vector<string> cities = {"București", "Cluj-Napoca", "Timișoara", "Iași", "Brașov"};
        vector<string> files = {"employees.csv","products.csv","orders.csv","recipes.csv","report.csv","events.csv"};//i will use recipes.csv for the products that are not premade. 
        //it will also interact with products.csv which contains the ingredients
        vector <string> headers = {"Name,Role,Starting hour,Ending hour,Salary\n",
                                    "Product name,Quantity,Price,Type\n",//type will be used to know if it is ingredient or part of menu
                                    "Customer name,Number of orders,Sum,Discount\n",
                                    "Recipes,Price/unit,(Ingredients,Quantity):,\n",
                                    header_report+"Profit,0,\n"
                                    "Expenses,0,\n",
                                    "Event,date,description,special products,cost,\n"
                                    };

        for (const auto& city : cities) {
            int header_index = 0;
            for (const auto& file : files) {
                string filePath = in_folder + "/" +city + "/" + file;
                
                try {
                    // Check if the file already exists
                    if (!filesystem::exists(filePath)) {
                        ofstream outFile(filePath);
                        ofstream f (filePath);
                        f<<headers[header_index];
                        if (outFile) {
                            cout << "File created successfully: " << filePath << endl;
                            outFile.close();
                        } else {
                            cerr << "Error creating file: " << filePath << endl;
                        }
                    } else {
                        //cout << "File already exists: " << filePath << endl;
                    }
                } catch (const exception& e) {
                    cerr << "Error creating file (" << filePath << "): " << e.what() << endl;
                }
                header_index++;
            }
            ifstream fin(in_folder + "/" + city + "/" + "report.csv");
            //renew the content of the report.csv file if a new day started;
            string date;//the first line in the report
            getline(fin, date);
            date = date + "\n";
            if(date!=header_report)
                {
                    fin.close();
                    ofstream fout(in_folder + "/" + city + "/" + "report.csv");
                    fout << header_report+"Profit,0,\n"+"Expenses,0,\n";//rewrite the file from zero for a new day
                    fout.close();
                }
                else
                    fin.close();
        }
    }
};


CreateFolders* CreateFolders::instance = nullptr;
once_flag CreateFolders::flag;

CreateFolders* CreateFolders::getInstance() {
    call_once(flag, []() {
        instance = new CreateFolders();
    });
    return instance;
}
//this function will be used by multiple classes in order to interact with products.csv easily
pair<float,float> changeQuantityInProducts(string city, string request, float change){//change represents a change in quantity.change can be negative or positive, depending on the action
    float final_price = -1;
    float rest_quantity;
        string filePath1 = in_folder + "/"+city + "/" + "products.csv";
        //firstly we need to check if we can find the order in products.csv if it's a premade good
        ifstream productsFilein(filePath1);
        string line,product,quantity,price,type;
        vector<string>lines;//vector of lines for overwriting the file
        getline(productsFilein,line);
        lines.emplace_back(line);
        bool ok_qu = true;
        while(getline(productsFilein,line)){
           // cout<<line;
            stringstream ss(line);
            getline(ss,product,',');
            getline(ss,quantity,',');
            getline(ss,price,',');
            getline(ss,type,',');

            
            if(product!=request){
                lines.emplace_back(line);
            }
            else{
                final_price = stof(price);
                //reconstructing the line with the new quantity
                //parsing quantity
                stringstream qq(quantity);
                float nr_quantity;
                string nr_quan;
                string unit;
                getline(qq,nr_quan,'[');
                getline(qq,unit,']');
                nr_quantity = stof(nr_quan);
                if(nr_quantity+change>=0)
                    nr_quantity = nr_quantity + change;
                    else
                    ok_qu = false;
                rest_quantity = nr_quantity;
                string new_quantity = to_string(nr_quantity);
                line = product+","+new_quantity+"["+unit+"]"+","+price+","+type+",";
                lines.emplace_back(line);
                

            }

        }
        
        //overwriting content
        ofstream productsFileout(filePath1);
        for (const auto& l : lines) {
        productsFileout << l << '\n'; 

        
    }
    productsFileout.close();
    if(ok_qu == false)
        final_price = -2;

        return make_pair(final_price,rest_quantity);
}

bool isToday(const string& dateStr) {
    
    auto now = chrono::system_clock::now();
    time_t current_time = chrono::system_clock::to_time_t(now);
    struct tm* now_tm = localtime(&current_time);

    struct tm eventDate = {};
    stringstream ss(dateStr);
    char delimiter;

    // format "dd/mm/yyyy"
    ss >> eventDate.tm_mday >> delimiter >> eventDate.tm_mon >> delimiter >> eventDate.tm_year;

    eventDate.tm_mon -= 1;
    eventDate.tm_year -= 1900;

    return (eventDate.tm_year == now_tm->tm_year) &&
           (eventDate.tm_mon == now_tm->tm_mon) &&
           (eventDate.tm_mday == now_tm->tm_mday);
}
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}


// bool isValidDate(const string& date) {
//     regex datePattern(R"(^(\d{2})/(\d{2})/(\d{4})$)");
//     smatch match;

//     if (!regex_match(date, match, datePattern)) {
//         return false;
//     }

//     int day = stoi(match[1]);
//     int month = stoi(match[2]);
//     int year = stoi(match[3]);

//     if (month < 1 || month > 12) {
//         return false;
//     }

//     int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//     if (isLeapYear(year) && month == 2) {
//         daysInMonth[1] = 29;
//     }

//     return day >= 1 && day <= daysInMonth[month - 1];
// }
bool isValidDate(const string& date) {
    int day, month, year;

    // Verificăm formatul utilizând un stringstream
    char delimiter1, delimiter2;
    stringstream ss(date);

    if (!(ss >> day >> delimiter1 >> month >> delimiter2 >> year) || delimiter1 != '/' || delimiter2 != '/') {
        return false; // Formatul nu este corect
    }

    // Validăm că ziua și luna nu au zerouri în față
    if (date.find("0") == 0 || date.find("/0") != string::npos) {
        return false; // Ziua sau luna începe cu 0
    }

    // Validăm ziua, luna și anul
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (year < 1900 || year > 2100) return false;

    // Validăm numărul de zile în funcție de lună
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (day > (isLeap ? 29 : 28)) return false;
    }

    return true;
}

string requestAndValidateDate() {
    string date;

    while (true) {
        try {
            cout << "Enter event date (DD/MM/YYYY):\n";
            getline(cin, date);

            if (!isValidDate(date)) {
                throw invalid_argument("Invalid date format or out-of-range values. Please try again.");
            }
            break;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    return date;
}
bool isHourInShift(int start, int end, int hour) {
    if (start <= end) {
        return hour >= start && hour < end;
    } 
    else {
        return (hour >= start && hour < 24) || (hour >= 0 && hour < end);
    }
}
void changeReport(string city,float price, string type){
    ifstream reportIn(in_folder + "/" + city + "/" + "report.csv");//add the new expenses for refilling the supply
                    string line_report_date,line_report_profit,line_report_expenses;
                    getline(reportIn,line_report_date);//first one is current date, second one profit, last one expense
                    getline(reportIn,line_report_profit);
                    getline(reportIn,line_report_expenses);

                    if(type=="expense"){
                        string expense;
                        stringstream ss(line_report_expenses);
                        getline(ss,expense,',');
                        getline(ss,expense,',');
                        float expense_float = stof(expense);
                        expense_float = expense_float + price;
                        expense = to_string(expense_float);
                        line_report_expenses = "Expenses,"+expense+",";
                    }
                    if(type == "profit"){
                        string profit;
                        stringstream ss(line_report_profit);
                        getline(ss,profit,',');
                        getline(ss,profit,',');
                        float profit_float = stof(profit);
                        profit_float = profit_float + price;
                        profit = to_string(profit_float);
                        line_report_profit = "Profit,"+profit+",";

                    }
                    if(type == "salary"){

                    }
    reportIn.close();
    ofstream reportOut(in_folder + "/" + city + "/" +"report.csv");
    reportOut<<line_report_date<<"\n";
    reportOut<<line_report_profit<<"\n";
    reportOut<<line_report_expenses<<"\n";
    reportOut.close();
}

//chain of responsibility design pattern
class Handler {
protected:
    shared_ptr<Handler> nextHandler;//pointers for sharing resources
    string city;
public:
    Handler():city(){city = "București";}
    Handler(const string& cityName) : city(cityName) {}
    virtual ~Handler() = default;

    void setNextHandler(shared_ptr<Handler> handler) {
        nextHandler = handler;
    }

    virtual void handleRequest(const string &customer_name, const vector<string>& requests) {
        if (nextHandler) {
            nextHandler->handleRequest(customer_name, requests);
        }
    }
};

class CustomerHandler : public Handler {
public:
    CustomerHandler(const string& cityName) : Handler(cityName) {}
    void checkMenu(){
        ifstream fin_recipes(in_folder + "/" + city + "/" + "recipes.csv");
        ifstream fin_products(in_folder + "/" + city + "/" + "products.csv");

   
    cout << MAGENTA << "*******************************" << RESET << endl;
    if(lang == "en")
        cout << MAGENTA <<"*           MENU             *" << RESET << endl;
        else
        cout<< MAGENTA << "*           MENIU            *" << RESET << endl;
    cout << MAGENTA << "*******************************" << RESET << endl;
    string line;
    int color = 0;
    getline(fin_recipes,line);
    while(getline(fin_recipes,line)) {
        // alternate colors
        string recipe,price;
        stringstream ss(line);
        getline(ss,recipe,',');
        getline(ss,price,',');
        if (color % 2 == 0) { 
            cout << GREEN; 
        } else {
            cout << BLUE;
        }
        color++;
        if(lang=="ro")
        cout << "* " << translate_to_romanian(recipe) << "   " << price << "  *" << RESET << endl;
        else
        cout << "* " << recipe << "   " << price << "  *" << RESET << endl;
    }
    fin_recipes.close();
    getline(fin_products,line);
    while(getline(fin_products,line)) {
        //colors that switch
        string product,price,quantity,type;
        stringstream ss(line);
        getline(ss,product,',');
        getline(ss,quantity,',');
        getline(ss,price,',');
        getline(ss,type,',');
        if(type=="1"){
        if (color % 2 == 0) { 
            cout << GREEN; 
        } else {
            cout << BLUE;
        }
        color++;
        if(lang == "en")
            cout << "* " << product << "   " << price << "  *" << RESET << endl;
            else
            cout << "* " << translate_to_romanian(product) << "   " << price << "  *" << RESET << endl;
        }
    }
    fin_recipes.close();


    cout << YELLOW << "*******************************" << RESET << endl;
}
    void handleRequest(const string& customer_name,const vector<string>& requests) override {
        //cout << "Customer makes a request: " << request << endl;
        if (nextHandler) {
            nextHandler->handleRequest(customer_name, requests);
        }
    }
};

class WaiterHandler : public Handler {
    private:
        queue<string>waiterQueue;
    public:
         WaiterHandler(const string& cityName) : Handler(cityName) {
        initializeWaiters();
    }

    void initializeWaiters() { //create queue of waiters
        time_t currentTime = time(nullptr); 
        tm* localTime = localtime(&currentTime);
        int hour = localTime->tm_hour;

        string filePath = in_folder +"/"+ city + "/" + "employees.csv";
        ifstream file(filePath);
        string line;

        if (!file.is_open()) {
            cerr << "Error: Could not open " << filePath << endl;
            return;
        }

        getline(file, line);
        while (getline(file, line)) {
            stringstream ss(line);
            string name, role, startHourStr, endHourStr;

            getline(ss, name, ',');
            getline(ss, role, ',');
            getline(ss, startHourStr, ',');
            getline(ss, endHourStr, ',');

            int startHour = stoi(startHourStr);
            int endHour = stoi(endHourStr);

            if (role == "waiter" && isHourInShift(startHour, endHour, hour)) {
                waiterQueue.push(name);
            }
        }

        file.close();
    }
        void handleRequest(const string& customer_name, const vector<string>& requests) override {
            //made a queue of waiters that are available at the current hour
            //if a waiter took an order he will be popped out of the queue and put in the back so the next one can come for the next order
            //i als took into consideration to put in the queue only tha waiters available at the current hour
            if(waiterQueue.empty()){
                if(lang == "en")
                cout<<"Sorry, but there are no available waiters at this time!Please try again later!\n";
                else
                cout<<"Ne pare rau dar nu sunt chelneri disponibili în acest monet! Te rugăm, încearcă mai târziu!\n";
                return;
            }
            string waiterName = waiterQueue.front();
            if(lang=="en")
                cout << "Waiter " << waiterName << " took your order: ";
                else
                cout<<"Chelnerul "<<waiterName<<" a preluat comanda: ";
            for (const auto& request : requests) {
                 cout<< request <<", ";
            }
            cout<<"\n";
            // Add the waiter back to the queue
            waiterQueue.push(waiterName);
                waiterQueue.pop();
                if (nextHandler) {
                    nextHandler->handleRequest(customer_name, requests);
                }
            }
};
//////////////////////////
//inheritance

class BaristaHandler : public Handler {
    private:
        //string city;
        queue<string>baristaQueue;
    public:
        BaristaHandler(const string& cityName) : Handler(cityName) {
        initializeBaristas();
    }

    void initializeBaristas() {
        time_t currentTime = time(nullptr); 
        tm* localTime = localtime(&currentTime);
        int hour = localTime->tm_hour;

        string filePath = in_folder + "/"+city + "/" + "employees.csv";
        ifstream file(filePath);
        string line;

        if (!file.is_open()) {
            cerr << "Error: Could not open " << filePath << endl;
            return;
        }

        getline(file, line);
        while (getline(file, line)) {
            stringstream ss(line);
            string name, role, startHourStr, endHourStr;

            getline(ss, name, ',');
            getline(ss, role, ',');
            getline(ss, startHourStr, ',');
            getline(ss, endHourStr, ',');

            int startHour = stoi(startHourStr);
            int endHour = stoi(endHourStr);

            if (role == "barista" && isHourInShift(startHour, endHour, hour)) {
                baristaQueue.push(name);
            }
        }

        file.close();
    }
    void handleRequest(const string& customer_name, const vector<string>& requests) override {
        string filePath1 = in_folder +"/"+ city + "/" + "products.csv";
        string filePath2 = in_folder +"/"+ city + "/" + "recipes.csv";

        if(baristaQueue.empty()){
                if(lang=="en")
                    cout<<"Sorry, but there are no available baristas at this time!Please try again later!\n";
                    else
                    cout<<"Ne pare rău, dar nu este nicio barista disponibilă în acest moment!Te rugăm, încearca mai târziu!\n";
                return;
            }
        string baristaName = baristaQueue.front();

        //cout<<city;

        // Add the barista back to the queue
        //the same logic as for the waiter one
        baristaQueue.push(baristaName);
            baristaQueue.pop();
            if (nextHandler) {
                nextHandler->handleRequest(customer_name, requests);
            }
        if(lang == "en")
            cout << "Barista " << baristaName << " took your order!\n";
        else
            cout<<"Barista "<<baristaName<<" a preluat comanda!\n";
        float total_sum = 0;
        for (const auto& request : requests) {
            
            bool available = true;
            
            pair<float,float> res =  changeQuantityInProducts(city,translate_to_english(request),-1);
                
            float final_price = res.first;
            if(final_price==-2)
                cout<<"Sorry, product out of stock!"<<"\n";
                else
            if(final_price!=-1){
                if(lang == "en")
                    cout<<"Hi, "<<customer_name<<"!Here is your: "<<request<<"⸜(｡˃ ᵕ ˂ )⸝♡\n";
                    else
                    cout<<"Hei, "<<customer_name<<"!Comanda ta: "<<request<<"⸜(｡˃ ᵕ ˂ )⸝♡\n";
                //total_sum=total_sum + final_price;
                }
                else{
                    final_price = 0;
                    //then maybe your desired product has to be prepared
                    //I will parse the recipe.csv in order to get the list of ingredients and their quantities
                    ofstream tempout(in_folder + "/" + city + "/" + "temp.txt");
                    bool found_recipe;
                    ifstream fiin(filePath1);
                    //copy the content of products.csv in order to restore it if there are not enough ingredients
                    string it;
                    while(getline(fiin,it)){
                        tempout<<it<<"\n";
                    }
                    tempout.close();
                    fiin.close();
                    string line,recipe,price;
                    ifstream fin(filePath2);
                    bool enough_ingredients = true;

                    getline(fin,line);//ignore first line
                    while(getline(fin,line)){
                        //cout<<line;
                        stringstream ss(line);
                        //cout<<line;
                        getline(ss,recipe,',');
                        //check if we can find the recipe in recipes.csv
                        
                        if(recipe == translate_to_english(request)){
                            //cout<<recipe;
                            getline(ss,price,',');//the next field is the price
                            string ingQuan;//ingredient + quantity
                            getline(ss,ingQuan,',');
                            bool ok = true; // bool that checks if we have all ingredients
                            while(ingQuan!=""){
                                string ingredient,quantity;//parsing this :  ,milk;0.3[l],
                                stringstream qq(ingQuan);
                                getline(qq,ingredient,';');
                                getline(qq,quantity,'[');
                                float q = stof(quantity);
                                
                                pair<float,float>res = changeQuantityInProducts(city,ingredient,-q);
                                //cout<<res.second<<endl;
                                getline(ss,ingQuan,',');
                                if(res.second<0){//can't have negative quantities so the recipe is not possible. Then products.csv has to be restored to original state
                                    
                                    
                                    enough_ingredients=false;
                                }

                            }
                            final_price=final_price+stof(price);
                            
                        }
                    }
                    fin.close();
                    if(enough_ingredients==false){
                        if(lang=="en")
                        cout<<"Sorry but the recipe is not available now! Please try another one or wait a moment...\n";
                        else
                        cout<<"Ne pare rău, dar rețeta nu este disponibilă! Te rugăm să încerci alta sau să așteptți un moment...\n";
                        available = false;
                        ofstream o(filePath1);
                        //if there are not enough ingredients restore the file to its original state
                        ifstream tempin(in_folder + "/" + city + "/" + "temp.txt");
                        string iter;
                        while(getline(tempin,iter)){
                            o<<iter+"\n";
                        }
                        o.close();
                    }
                    else{
                    if(lang == "en")
                        cout<<"Hi, "<<customer_name<<"!Here is your: "<<request<<"(∗´ര ᎑ ര`∗)\n";
                    else
                        cout<<"Hei, "<<customer_name<<"!Aici este comanda ta: "<<request<<"(∗´ര ᎑ ര`∗)\n";
                    
                    }
                    string filename = in_folder +"/"+ city + "/"+"temp.txt";
                    remove(filename.c_str());
                }
                    

                //if request is valid the we can keep evidence of his name in orders file
                //at first we need to check if the customer is already in the file
                    //if the customer is found nr_orders and price has to be incremented
                    ifstream finOrders(city + "/" + "orders.csv");
                    bool ok = false;//ok to check if the customer is already in the file
                    string linee;
                    vector<string>lines;
                    getline(finOrders,linee);//ignore header
                    lines.emplace_back(linee);
                    while(getline(finOrders,linee)){
                        //cout<<linee<<endl;
                        stringstream ss(linee);
                        string customer;
                        getline(ss,customer,',');
                        if(customer == customer_name){
                            //cout<<customer<<" "<<customer_name<<endl;
                            string orders, sum, discount;
                            getline(ss,orders,',');
                            int ord = stoi(orders);
                            ord++;
                            orders = to_string(ord);

                            getline(ss,sum,',');
                            float suma = stof(sum);
                            suma = suma + final_price;
                            sum = to_string(suma);

                            getline(ss,discount,',');
                            int discount_int = stoi(discount);
                            if(discount_int > 0)
                                cout<<"Congrats! You have a discount of "<<discount_int<<"%!";
                            if(available){
                                cout<<"Price:"<<final_price - final_price*discount_int/100<<"\n~~~~~~~~~~~~~~~~~\n";//apply the discount obtained from the prevoius order
                                total_sum= total_sum + (final_price - final_price*discount_int/100);}
                            if(ord >= 10 && suma >= 200){
                                discount_int = 8;
                            }
                            else
                            if(ord >= 10 && ord <20)
                                discount_int = 5;
                            else
                            if(suma >= 200)
                                discount_int = 5;
                            else
                            if(ord>=20 && ord <30)
                                discount_int = 4;
                            else
                            if(ord>=30)
                                discount_int = 7;
                            
                            discount_int = 0; //we need to make sure that the same discount is applied only once
                            ok = true;//customer is found
                        
                            discount = to_string(discount_int);
                            linee = customer + "," + orders + "," + sum + "," + discount +",";
                        }
                        lines.emplace_back(linee);

                    }
                    
                    finOrders.close();
                    if(ok == true){//then rewrite the entire file using the vector of lines
                    
                        ofstream foutOrders(city+"/"+"orders.csv");

                    for (const auto& l : lines) {
                            foutOrders << l << '\n'; 
                    }
                    foutOrders.close();

                    }
                    else
                    {
                
                            //then just print the initial price and append the new customer in orders.csv
                            string filePath = city + "/" + "orders.csv";
                            ofstream out(filePath, ios::app); //append the new line with ios::app
                            out<<customer_name<<","<<"1,"<<final_price<<",0,\n";
                            out.close();
                            cout<<"Price: "<<final_price<<"\n";
                            total_sum = total_sum + final_price;
                        

                    }
                
    
        
                    
                
        
        }
        cout<<"================\n";
        if(lang == "en")
            cout<<"Total price:\n";
            else
            cout<<"Preț total:\n";
        cout<<total_sum<<"\n";
        cout<<"================\n";
        changeReport(city,total_sum,"profit");
    }
};
//facade design pattern that helps the manager to handle product supply and employees file
//subsystem 1
class Employees{
    public:
        void displayEmployees(string city){
            
            cout<<"Employees from "<<city<<" cafe!\n";
            string filePath = in_folder +"/"+ city + "/" + "employees.csv";
            ifstream fin;
            fin.open(filePath);
            string line;

            while(getline(fin,line)){
                cout<<line<<"\n";
            }
        }

        void deleteEmployee(string city, string name){
            string filePath = in_folder + "/"+city + "/" + "employees.csv";
            ifstream fin(filePath);
            ofstream temp("temp_empl.txt");
            string line;

            while(getline(fin,line)){
                stringstream ss(line);
                string first_word;
                getline(ss,first_word,',');
                if(first_word != name){
                    temp<< line << "\n";
                }

            }
            temp.close();
            fin.close();
            ifstream tempin("temp_empl.txt");
            ofstream fout(filePath);
            while(getline(tempin,line)){
                fout<<line<<"\n";
            }
            tempin.close();
            fout.close();
            string filename = "temp_empl.txt";
            remove(filename.c_str());
        }
        void addEmployee(string city, string name,string role, int starting_hour, int ending_hour, float salary){
            string filePath = in_folder +"/"+ city + "/" + "employees.csv";
            ofstream out(filePath, ios::app); //append the new line with ios::app
            out<<name<<","<<role<<","<<starting_hour<<","<<ending_hour<<","<<salary<<",\n";
            out.close();


        }
        // void checkFreeInterval(string city){
        //     string filePath = city + "/" + "employees.csv";
            
        // }

        
};

//subsystem 2
class Products{
    public:
        void checkProductSupply(string city){
            string filePath = in_folder +"/"+ city + "/" + "products.csv";
            cout<<"Products available in "<<city<<" cafe:\n";
            ifstream fin;
            fin.open(filePath);
            string line;

            while(getline(fin,line)){
                cout<<line<<"\n";
            }
            
        }
        void addProduct(string city, string product, string quantity, float price, int type){
            string filePath = in_folder +"/"+ city + "/" + "products.csv";
            ofstream out(filePath, ios::app); //append the new line with ios::app
            out<<product<<","<<quantity<<","<<price<<","<<type<<","<<"\n";
            string no_unit_quantity;
            stringstream ss(quantity);
            getline(ss,no_unit_quantity,'[');
            changeReport(city,stof(quantity)*price,"expense");
        }

};
//subsystem 3 of facade
class Event{
    protected:
        string city;
        string event;
        string date;
        vector<string> special_products;
        string description;
        int cost;
    public:
        Event(){
            event = "Cafe's anniversary";
            date = "28/01/2024";
            special_products={"icy donut;100[pcs];15","Snow white cake;40[pcs];16","Oriental juice;2[l];13"};
            description="Cafe's anniversary";
            cost = 2000;
        }
        
        Event(string _event, string _date, vector<string> _special_products, string _description, int _cost, string city){
            event = _event;
            date = _date;
            special_products=_special_products;
            description=_description;
            cost=_cost;
        }
        
        void addToEventsFile(string city){
            ofstream fout(in_folder + "/" + city + "/" + "events.csv", ios::app);
            fout<<event<<",";
            fout<<date<<",";
            for (const auto& product : special_products) {
                            fout << '('<<product << ')'; 
                    }
            fout<<",";
            fout<<description<<",";
            fout<<cost<<",\n";

            
        }
        void addSpecialProducts(){
            //function to add special products to products.csv
            //they will be recognized by their type which is the date of the event and only that particular date would enable them
            ifstream fin(in_folder + "/" + city + "/" + "products.csv");
        }
};
//facade
//the manager is responsible for handling waiters,baristas and products
class Manager{
    private:
        Employees employees;
        Products products;
        Event events;
    public:
        string city;
        //default constructor
        Manager(){
            city = "București";
        }
        //constructor for the attribute city
        Manager(string City){
            city = City;
        }
        //the menu for manager will have 1.Handle employees/n2.Handle products/ng3.New Recipe
        void handleEmployees(){
            int option;
    
        while (true) {
            try {
                cout << "\n1. Display Employees\n";
                cout << "2. Delete Employee\n";
                cout << "3. Add Employee\n";
                cout << "0. Exit\n";
                cout << "Choose an option: ";
                cin >> option;

                if (option == 0) {
                    cout << "Exiting program...\n";
                    break;
                }

                if (option == 1) {
                    
                    employees.displayEmployees(city);
                } else if (option == 2) {
                    string name;
                    cout << "Enter the name of the employee to delete: ";
                    cin >> name;
                    employees.deleteEmployee(city, name);
                } else if (option == 3) {
                    string name, role;
                    int start_hour, end_hour;
                    cout << "Enter employee name: ";
                    cin >> name;
                    cout << "Enter employee role: ";
                    cin >> role;
                    cout << "Enter start hour: ";
                    cin >> start_hour;
                    cout << "Enter end hour: ";
                    cin >> end_hour;
                    float salary;
                    cout<<"Salary: ";
                    cin >> salary;
                    employees.addEmployee(city, name, role, start_hour, end_hour, salary);
                } else {
                    throw runtime_error("Invalid option. Please choose a valid option.");
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
    }

                    
            }
        void HandleProducts(){
            int option;
            while(true){
                try{
                    cout << "\n1. Display Products\n";
                    cout << "2. Add Product\n";
                    cout << "3.Refill supply\n";
                    cout << "0. Exit\n";
                    cout << "Choose an option: ";
                    cin >> option;

                    if (option == 0) {
                        cout << "Exiting program...\n";
                        break;
                    }
                    if(option == 1)
                        products.checkProductSupply(city);
                        else
                    if(option == 2){
                        cin.ignore();
                        string product;
                        
                        cout<<"Product name:\n";
                        getline(cin,product);
                       
                        string quantity;
                        regex quantityPattern(R"(^\d+(\.\d+)?\[(pcs|l|kg)\]$)"); //regex for validating if data has the right pattern quantity[unit]

                        while (true) {
                            try {
                                cout << "Quantity: Please enter a valid format! (ex: 20[pcs], 10[l], 15[kg])\n";
                                cin >> quantity;

                                if (!regex_match(quantity, quantityPattern)) {
                                    throw invalid_argument("Invalid format. Please use the format like 20[pcs], 10[l], or 15[kg].");
                                }
                                //if the pattern is valid we can continue reading data so we exit the loop
                                break;
                            } catch (const exception& e) {
                                cout << "Error: " << e.what() << endl;
                            }
                        }
                        float price;
                        cout<<"Price:\n";
                        cin>>price;
                        int type;
                        cout<<"Type:(0 for ingredient, 1 for product that will appear on the MENU):\n";//there woul pe special type as a date in format DD/MM/YYYY  for events
                        cin>>type;
                        products.addProduct(city,product,quantity,price,type);
                        if(type==1){
                        cin.ignore();
                         cout<<"Translate product name in romanian:\n";
                        string trans;
                        getline(cin,trans);
                        addTranslate(product,trans);
                        }
                        

                    }
                    else if(option == 3){
                        //refill quantity
                        cin.ignore();
                        string product;
                        cout<<"Refill supply:\n";
                        cout<<"Product name:\n";
                        getline(cin,product);
                        string quantity;
                        regex quantityPattern(R"(^\d+(\.\d+)?\[(pcs|l|kg)\]$)"); //regex for validating if data has the right pattern quantity[unit]

                        while (true) {
                            try {
                                cout << "How much would you like to add? Quantity: Please enter a valid format! (ex: 20[pcs], 10[l], 15[kg])\n";
                                cin >> quantity;

                                if (!regex_match(quantity, quantityPattern)) {
                                    throw invalid_argument("Invalid format. Please use the format like 20[pcs], 10[l], or 15[kg].");
                                }
                                //if the pattern is valid we can continue reading data so we exit the loop
                                break;
                            } catch (const exception& e) {
                                cout << "Error: " << e.what() << endl;
                            }
                        }
                    pair<float,float>res=changeQuantityInProducts(city, product, stof(quantity));
                    changeReport(city,res.first*stof(quantity),"expense");
                    

                    }
                    else{
                         throw runtime_error("Invalid option. Please choose a valid option.");
                    }

                }
                catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            }
                

        }
        void newRecipes(){
            string name;
            cin.ignore();
            cout<<"What's the name of the recipe?\n";
            getline(cin,name);
            cout<<"Translate it in romanian:\n";
            string translate_name;
            getline(cin,translate_name);
            addTranslate(name,translate_name);
            string filePath = in_folder + "/"+city + "/" + "recipes.csv";
            ofstream out(filePath, ios::app); //append the new line with ios::app
            out<<name<<",";
            float price;
            cout<<"Price:\n";
            cin>>price;
            out<<price<<",";
            int number_of_ingredients;
            cout<<"Number of ingredients:\n";
            cin>>number_of_ingredients;
           cin.ignore();
            for(int i = 0; i < number_of_ingredients; i++){
                cout<<"Ingredient:\n";
                string ingredient;
                getline(cin,ingredient);
    
                string quantity;
                            regex quantityPattern(R"(^\d+(\.\d+)?\[(pcs|l|kg)\]$)"); //regex for validating if data has the right pattern quantity[unit]

                            while (true) {
                                try {
                                    cout << "Quantity: Please enter a valid format! (ex: 20[pcs], 10[l], 15[kg])\n";
                                    cin >> quantity;
                                    cin.ignore();

                                    if (!regex_match(quantity, quantityPattern)) {
                                        throw invalid_argument("Invalid format. Please use the format like 20[pcs], 10[l], or 15[kg].");
                                    }
                                    //if the pattern is valid we can continue reading data so we exit the loop
                                    break;
                                } catch (const exception& e) {
                                    cout << "Error: " << e.what() << endl;
                                }
                            }
                    out<< ingredient<<";"<<quantity<<",";
                
                        
            }
            out<<endl;
            
        }
        void HandleEvent(){ //this function may be used for future events but it is also used to update todays event
            string event;
            vector<string> special_products;
            string description, product;
            int cost;

            cout << "Event name:\n";
            cin.ignore();
            getline(cin, event);

            cout << "Event date (DD/MM/YYYY): without zero in front!!!\n";
            string date = requestAndValidateDate();
            

            cout << "Enter description:\n";
            getline(cin, description); 
            cout <<"Translate the description in Romanian:\n";
            string trans_descr;
            getline(cin,trans_descr);
            addTranslate(description,trans_descr);

            cout << "Enter cost of event:\n";
            cin >> cost;
            cin.ignore();
           
            regex productPattern(R"(^[a-zA-Z\s]+;\d+\[(pcs|l|kg)\];\d+(\.\d+)?$)");

                cout << "Enter special products in the format: name;quantity[unit];price/unit (type 'done' to finish):" << endl;
        while (true) {
            try {
                cout<<"Product name:\n";
                getline(cin, product);
                stringstream pp(product);
                string prName;
                getline(pp,prName,';');
                
                if (product == "done") {
                    break;
                }else{
                    string trans_product;
                cout<<"Translate product name in Romanian\n";
                getline(cin,trans_product);
                addTranslate(prName,trans_product);
                }

                if (!regex_match(product, productPattern)) {
                    throw invalid_argument("Invalid format! Use the format: name;quantity[unit];price/unit, where unit is one of [pcs], [l], or [kg].");
                }

                special_products.push_back(product);
            } catch (const exception &e) {
                cout << "Error: " << e.what() << endl;
                cout << "Please try again." << endl;
            }
        }
    events = Event(event,date,special_products,description,cost,city);
    events.addToEventsFile(city);
    
        }
};
class EventProduct{
    public:
        string name;
        float quantity;
        string unit;
        float price;
        EventProduct(){
            
        }
    public:
        EventProduct(string _name, float _quantity, string _unit, float _price){
            name = _name;
            quantity = _quantity;
            unit = _unit;
            price=_price;
        }
        void productDisplay(){
            if(lang == "ro")
            cout<<translate_to_romanian(name)<<": "<<quantity<<unit<<" preț: "<<price<<endl;
            else
            cout<<name<<": "<<quantity<<unit<<" price: "<<price<<endl;
        }

};



class TodaysEvent {
private:
    static unique_ptr<TodaysEvent> instance;
    static mutex mtx;
    string city;
    string eventName;
    string description;
    vector<EventProduct> products;
    double cost;
    struct tm eventDate;

    // private constructor for singleton pattren
    TodaysEvent(const string& _city) : city(_city) {
        string filePath = in_folder + "/"+city + "/" + "events.csv";
        ifstream file(filePath);

        string line;
        bool eventFound = false;
        getline(file,line);
        while (getline(file, line)) {
            stringstream ss(line);
            string eventDateStr, eventDesc, productsStr, costStr;
            //cout<<line;
            getline(ss, eventName, ',');
            getline(ss, eventDateStr, ',');
            getline(ss,productsStr,',');
            getline(ss, eventDesc, ',');
            getline(ss, costStr, ',');

            // getline(ss, costStr);
            // cout<<"name:"<<eventName<<endl;
            // cout<<"date: "<<eventDateStr<<endl;
            // cout<<"prdutcs: "<<productsStr<<endl;
            // cout<<"description: "<<eventDesc<<endl;
            // cout<<"cost: "<<costStr<<endl;
            description = eventDesc;

            // Folosim get_time pentru a analiza data
            stringstream dateStream(eventDateStr);
            dateStream >> get_time(&eventDate, "%d/%m/%Y");
            

            if (isEventToday()) {
                eventFound = true;
                stringstream productsStream(productsStr);
                string productData;
                while (getline(productsStream, productData, ')')) {
                    
                    stringstream ss(productData);
                    string productName,quatityStr,unitStr,priceStr;
                    getline(ss,productName,';');
                    getline(ss,quatityStr,'[');
                    getline(ss,unitStr,']');
                    getline(ss,priceStr,')');
                    //cout<<priceStr<<endl;
                    float quantity = stod(quatityStr);
                    float price = stod(priceStr.erase(0,1)); 
                    //cout<<price<<endl;
                    products.push_back(EventProduct(productName.substr(1), quantity, unitStr, price));
             
                }
                //cout<<costStr;
                cost = stod(costStr);
                
                break;
            }
        }

        file.close();

        if (!eventFound) {
            cout << "No event today: ";
            eventName="NO event";
            description="none";

        }
        else changeReport(city,cost,"expense");
    }
    // ~TodaysEvent() {
    //     cout << "TodaysEvent instance destroyed." << endl;
    // }

    bool isEventToday() {
        auto now = chrono::system_clock::now();
        time_t current_time = chrono::system_clock::to_time_t(now);
        struct tm* now_tm = localtime(&current_time);
        return (eventDate.tm_year == now_tm->tm_year) &&
               (eventDate.tm_mon == now_tm->tm_mon) &&
               (eventDate.tm_mday == now_tm->tm_mday);
    }

public:
    //delete copy operators so we don't dup[licate]
    TodaysEvent(const TodaysEvent&) = delete;
    TodaysEvent& operator=(const TodaysEvent&) = delete;

    // getting unique instance
    static TodaysEvent& getInstance(const string& _city) {
        lock_guard<mutex> lock(mtx);
        if (!instance) {
            instance.reset(new TodaysEvent(_city));  
        }
        return *instance;
    }

    void updateEvent(const string& _event_name, const string& _description, vector<EventProduct> _products, double _cost) {
        eventName = _event_name;
        description = _description;
        products = _products;
        cost = _cost;
    }
    void order(string name, vector<string> requests) {
    string filePath = in_folder + "/" + city + "/" + "events.csv";
    string tempFilePath = in_folder + "/" + city + "/" + "events_temp.csv";
    string ordersFilePath = in_folder + "/" + city + "/" + "orders.csv";

    float price = 0;

    // Calculate total price and update product quantities
    for (const string &request : requests) {
        bool found = false;
        for (auto &product : products) {
            if (product.name == request || product.name == translate_to_english(request)) {
                if (product.quantity > 0) {
                    price += product.price;
                    product.quantity--;
                    changeReport(city, product.price, "profit");
                    found = true;
                } else {
                    cerr << "Not enough stock for product: " << request << endl;
                }
                break;
            }
        }
        if (!found) {
            cerr << "Product not found: " << request << endl;
        }
    }

    // Read and update orders.csv
    ifstream finOrders(ordersFilePath);
    vector<string> lines;
    bool customerFound = false;

    if (!finOrders.is_open()) {
        cerr << "Error opening orders file." << endl;
        return;
    }

    string line;
    getline(finOrders, line); // Read header
    lines.push_back(line);

    while (getline(finOrders, line)) {
        stringstream ss(line);
        string customer, orders, sum, discount;

        getline(ss, customer, ',');
        getline(ss, orders, ',');
        getline(ss, sum, ',');
        getline(ss, discount, ',');

        if (customer == name) {
            customerFound = true;

            // Update orders and sum
            int ord = stoi(orders) + 1;
            float suma = stof(sum) + price;

            // Handle discount for special conditions
            int discountInt = stoi(discount);
            if (requests.size() > 5 || price >= 100) {
                discountInt = 10; // 10% discount for the next order
                if (lang == "en")
                    cout << "Discount: 10% for the next order\n";
                else
                    cout << "Reducere: 10% la urmatoarea comanda\n";
            }

            line = customer + "," + to_string(ord) + "," + to_string(suma) + "," + to_string(discountInt) + ",";
        }

        lines.push_back(line);
    }

    finOrders.close();

    if (!customerFound) {
        // Add new customer if not found
        lines.push_back(name + ",1," + to_string(price) + ",0,");
        if (lang == "en")
            cout << "Hi, " << name << "! Today's event order cost is: " << price << "\n";
        else
            cout << "Hei, " << name << "! Comanda ta din cadrul evenimentului este de: " << price << "\n";
    }

    // Rewrite orders.csv
    ofstream foutOrders(ordersFilePath);
    for (const auto &l : lines) {
        foutOrders << l << '\n';
    }
    foutOrders.close();

    // Update events.csv with new product quantities
    ifstream inputFile(filePath);
    ofstream tempFile(tempFilePath);

    if (!inputFile.is_open() || !tempFile.is_open()) {
        cerr << "Error opening file for reading or writing: " << filePath << " or " << tempFilePath << endl;
        return;
    }

    bool headerProcessed = false;
    while (getline(inputFile, line)) {
        if (!headerProcessed) {
            tempFile << line << endl;
            headerProcessed = true;
            continue;
        }

        stringstream ss(line);
        string event, date, description, productsStr, costStr;

        getline(ss, event, ',');
        getline(ss, date, ',');
        getline(ss, productsStr, ',');
        getline(ss, description, ',');
        getline(ss, costStr);

        if (isEventToday()) {
            string updatedProductsStr;
            for (const auto &product : products) {
                updatedProductsStr += "(" + product.name + ";" +
                                      to_string(static_cast<int>(product.quantity)) + "[" + product.unit + "];" +
                                      to_string(static_cast<int>(product.price)) + ")";
            }
            tempFile << event << "," << date << "," << updatedProductsStr << "," << description << "," << costStr << endl;
        } else {
            tempFile << line << endl;
        }
    }

    inputFile.close();
    tempFile.close();

    if (remove(filePath.c_str()) != 0 || rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        cerr << "Error replacing original file with updated file." << endl;
    }
}

    void printEventDetails() const {
        if(lang == "ro"){
        cout << "Nume eveniment: " << eventName << endl;
        cout << "Description: " << translate_to_romanian(description) << endl;
        cout << "Cost: " << cost << endl;
        cout << "Produse: " << endl;
        for (const auto& product : products) {
            cout << "- " << translate_to_romanian(product.name) << ": " << product.quantity << " " << product.unit << ", preț: " << product.price << endl;
        }
}
        else{
        cout << "Event Name: " << eventName << endl;
        cout << "Description: " << description << endl;
        cout << "Cost: " << cost << endl;
        cout << "Products: " << endl;
        for (const auto& product : products) {
            cout << "- " << product.name << ": " << product.quantity << " " << product.unit << ", price: " << product.price << endl;
        }
        }
    }
};

unique_ptr<TodaysEvent> TodaysEvent::instance = nullptr;
mutex TodaysEvent::mtx;

//Observer Pattern for manager to get notifications about employees and products
template <typename T>
class Notifier {
public:
    void notify(const T& message) {
        notifications.push_back(message);
    }

    void displayNotifications() {
        if (notifications.empty()) {
            cout << "No notifications.\n";
            return;
        }

        cout <<MAGENTA<< "\n=== Notifications ===\n"<<RESET;
        for (const auto& notification : notifications) {
            displayMessage(notification);
        }
        notifications.clear();//empty notifications after displaying
    }

private:
    vector<T> notifications;

    void displayMessage(const T& message) {
        // Default behavior pentru tipurile generice (poate fi personalizat pentru alte tipuri)
        cout << "- " << message << endl;
    }
};
class EmployeesNotifier {
    string city;
    Notifier<string>* notifier; // Folosim Notifier-ul templatizat

public:
    EmployeesNotifier(const string& city, Notifier<string>* notifier) : city(city), notifier(notifier) {}
    //~EmployeesNotifier();

    void checkRoleCoverage() {
        string filePath = in_folder + "/"+city + "/employees.csv";
        ifstream file(filePath);

        if (!file.is_open()) {
            notifier->notify("Error: Could not open file " + filePath);
            return;
        }

        vector<bool> baristaCoverage(24, false);
        vector<bool> waiterCoverage(24, false);

        string line;
        getline(file, line);

        while (getline(file, line)) {
            stringstream ss(line);
            string name, role, startHourStr, endHourStr;

            getline(ss, name, ',');
            getline(ss, role, ',');
            getline(ss, startHourStr, ',');
            getline(ss, endHourStr, ',');

            int startHour = stoi(startHourStr);
            int endHour = stoi(endHourStr);

            if (role == "barista") {
                markCoverage(baristaCoverage, startHour, endHour);
            } else if (role == "waiter") {
                markCoverage(waiterCoverage, startHour, endHour);
            }
        }

        file.close();

        notifyGaps("Barista", baristaCoverage);
        notifyGaps("Waiter", waiterCoverage);
    }

private:
    void markCoverage(vector<bool>& coverage, int startHour, int endHour) {
        if (startHour <= endHour) {
            for (int i = startHour; i < endHour; i++) {
                coverage[i] = true;
            }
        } else { 
            for (int i = startHour; i < 24; i++) {
                coverage[i] = true;
            }
            for (int i = 0; i < endHour; i++) {
                coverage[i] = true;
            }
        }
    }

    void notifyGaps(const string& role, const vector<bool>& coverage) {
        int start = -1;
        // for(int i = 0; i < 24; i++)
        //     cout<<coverage[i];
        for (int i = 0; i < 24; i++) {
            if (!coverage[i] && start == -1) {
                start = i;
            } else if (coverage[i] && start != -1) {
                notifier->notify(role + " gap: " + to_string(start) + ":00 - " + to_string(i) + ":00");
                start = -1;
            }
        }

        if (start != -1) {
            notifier->notify(role + " gap: " + to_string(start) + ":00 - 00:00");
        }
    }
};
class ProductsNotifier {
private:
    string city;
    Notifier<string>* notifier;

    //function to extract quantity from n[unit]
    float parseQuantity(const string& quantityStr) {
        regex pattern(R"((\d+(\.\d+)?))");
        smatch match;
        if (regex_search(quantityStr, match, pattern)) {
            return stof(match[1].str());
        }
        throw invalid_argument("Invalid quantity format: " + quantityStr);
    }

public:
    ProductsNotifier(const string& city, Notifier<string>* notifier)
        : city(city), notifier(notifier) {}

    void checkProductQuantities() {
        string filePath = in_folder +"/"+ city + "/products.csv";
        ifstream file(filePath);

        if (!file.is_open()) {
            notifier->notify("Error: Unable to open " + filePath);
            return;
        }

        string line;
        getline(file, line); 
        while (getline(file, line)) {
            istringstream lineStream(line);
            string productName, quantityStr, priceStr, typeStr;
            getline(lineStream, productName, ',');
            getline(lineStream, quantityStr, ',');
            getline(lineStream, priceStr, ',');
            getline(lineStream, typeStr, ',');
            
            try {
                float quantity = parseQuantity(quantityStr);

                if (quantity <= 1) {
                    notifier->notify("Low stock for product: " + productName + " (" + quantityStr + ") in " + city);
                }
            } catch (const exception& e) {
                notifier->notify("Error processing product: " + productName + ". " + e.what());
            }
        }

        file.close();
    }
};
void generateReport(string city){
    ifstream fin_report(in_folder + "/" + city+"/report.csv");
    ifstream fin_employees(in_folder + "/" + city+"/employees.csv");
    cout<<GREEN<<"_________REPORT_________\n"<<RESET;
    string line;
    getline(fin_report,line);
    cout<<line<<"\n";
    if(isToday(line)){
        getline(fin_report,line);
        cout<<line<<"\n";
        getline(fin_report,line);
        cout<<line<<"\n";
    }
    else
    cout<<"No changes have been made today!\n";
    cout<<GREEN<<"_______EMPLOYEES________\n"<<RESET;
    while(getline(fin_employees,line)){
        cout<<line<<"\n";
    }
    cout<<GREEN<<"________________________\n"<<RESET;
}

// command pattern for processing different cases of user's interactions with the system
//the difference between command pattern and facade is that command pattern converts requests into objects
//we may need different types of objects, for manager, customer...
//this class will constanly interact with the class TodayEvent
class UserInterface {
public:
    void CustomerMenu(shared_ptr<CustomerHandler> customer, TodaysEvent& todays_event) {
    int choice;

    
    cout << "Select your lang / Selectati limba:\n";
    cout << "1. English\n";
    cout << "2. Romana\n";
    cout << "Enter your choice: ";
    cin >> choice;

    if (choice == 1) {
        lang = "en";
    } else if (choice == 2) {
        lang = "ro";
    } else {
        cout << "Invalid choice. Defaulting to English.\n";
        lang = "en";
    }

    while (true) {
        try {
            if (lang == "en") {
                cout << "\nCustomer Menu:\n";
                cout << "1. Check Menu\n";
                cout << "2. Place Order\n";
                cout << "3. Today's event\n";
                cout << "4. Order special product from event\n";
                cout << "0. Back to Main Menu\n";
            } else {
                cout << "\nMeniul clientului:\n";
                cout << "1. Verifica meniul\n";
                cout << "2. Plaseaza comanda\n";
                cout << "3. Evenimentul zilei\n";
                cout << "4. Comanda produs special din eveniment\n";
                cout << "0. Inapoi la meniul principal\n";
            }

            cout << (lang == "en" ? "Enter your choice: " : "Introduceti optiunea: ");
            cin >> choice;

            if (choice == 1) {
                customer->checkMenu();
            } else if (choice == 2) {
                cout << (lang == "en" ? "Enter your name: " : "Introduceti numele: ");
                string name;
                cin.ignore();
                getline(cin, name);

                vector<string> orderItems;
                cout << (lang == "en" ? "Enter your order (type 'done' to finish):\n" : "Introduceti comanda (scrieti 'done' pentru a termina):\n");
                string item;
                while (true) {
                    cout << (lang == "en" ? "Item: " : "Produs: ");
                    getline(cin, item);
                    if (item == "done") break;
                    orderItems.push_back(item);
                }
                customer->handleRequest(name, orderItems);
            } else if (choice == 3) {
                todays_event.printEventDetails();
            } else if (choice == 4) {
                string name;
                cout << (lang == "en" ? "What's your name?\n" : "Care este numele tau?\n");
                cin.ignore();
                getline(cin,name);
                cout << (lang == "en" ? "Enter special products from event: when finished type 'done'\n" : "Introduceti produse speciale din eveniment: cand terminati, scrieti 'done'\n");
                vector<string> requests;
                string request;
                //cin.ignore();
                getline(cin, request);
                while (request != "done") {
                    requests.push_back(request);
                    getline(cin, request);
                }
                todays_event.order(name,requests);
            } else if (choice == 0) {
                break;
            } else {
                throw invalid_argument(lang == "en" ? "Invalid choice. Please enter a valid option." : "Optiune invalida. Va rugam sa introduceti o optiune valida.");
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
}
    void ManagerMenu(Manager& manager, TodaysEvent& todays_event, string city) {
        int choice;
        Notifier<string> notifier; 
        EmployeesNotifier employees_notifications(city, &notifier);
        employees_notifications.checkRoleCoverage();
        ProductsNotifier productsNotifier(city, &notifier);
        productsNotifier.checkProductQuantities();
    
        notifier.displayNotifications();
        while (true) {
            try {
                cout << "\nManager Menu:\n";
                cout << "1. Manage Employees\n";
                cout << "2. Handle Products\n";
                cout << "3. Add New Recipes\n";
                cout << "4. Change Product Quantity\n";
                cout << "5. Handle Event\n";
                cout << "6. Generate Report\n";
                cout << "7. Back to Main Menu\n";
                cout << "Enter your choice: ";
                cin >> choice;

                if (choice == 1) {
                    manager.handleEmployees();
                } else if (choice == 2) {
                    manager.HandleProducts();
                } else if (choice == 3) {
                    manager.newRecipes();
                } else if (choice == 4) {
                    cout << "Enter product name: ";
                    string product;
                    float quantity;
                    cin.ignore();
                    getline(cin, product);
                    cout << "Enter quantity: ";
                    cin >> quantity;
                    cout << "Changing quantity of " << product << " to " << quantity << ".\n";
                } else if (choice == 5) {
                    manager.HandleEvent();
                } else if (choice == 6) {
                    generateReport(city);
                }
                else if (choice == 7) {
                    break;
                }
                 else {
                    throw invalid_argument("Invalid choice. Please enter a valid option.");
                }
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
    }

    string chooseCity() {
        int choice;
        while (true) {
            try {
                cout << "\nChoose a city:\n";
                cout << "1. Brasov\n";
                cout << "2. Bucuresti\n";
                cout << "3. Cluj-Napoca\n";
                cout << "4. Iasi\n";
                cout << "5. Timisoara\n";
                cout << "Enter your choice (1-5): ";
                cin >> choice;

                switch (choice) {
                    case 1: return "Brașov";
                    case 2: return "București";
                    case 3: return "Cluj-Napoca";
                    case 4: return "Iași";
                    case 5: return "Timișoara";
                    default:
                        throw invalid_argument("Invalid choice. Please select a valid number between 1 and 5.");
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
    }

    void mainMenu() {
        
        int roleChoice;
        while (true) {
            try {
                cout << "\nMain Menu:\n";
                cout << "1. Customer\n";
                cout << "2. Manager\n";
                cout << "3. Exit\n";
                cout << "Enter your choice: ";
                //cin.ignore();
                cin >> roleChoice;
                if (roleChoice == 3) {
                    cout << "Exiting...\n";
                    exit(0);
                }
                string city = chooseCity();
                 TodaysEvent& todays_event = TodaysEvent::getInstance(city);
    
                if (roleChoice == 1) {
                    auto customer = make_shared<CustomerHandler>(city);
                    auto waiter = make_shared<WaiterHandler>(city);
                    auto barista = make_shared<BaristaHandler>(city);

                    // Set up the chain of responsibility
                    customer->setNextHandler(waiter);
                    waiter->setNextHandler(barista);

                    CustomerMenu(customer, todays_event);
                } else if (roleChoice == 2) {
                    
                    //1cout<<city;
                    Manager manager(city);
                    ManagerMenu(manager, todays_event, city);
                } else {
                    throw invalid_argument("Invalid choice. Please enter 1, 2, or 3.");
                }
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
    }
};

int main(int argc, char *argv[]){
    if(argv[1]!=NULL){
    in_folder = argv[1];}
    else
    in_folder="Data";
    // out_folder = argv[2];
    // obtains unique instance of singleton
    CreateFolders* create_folders = CreateFolders::getInstance();

    //creates folders if they don't already exist
    create_folders->createDirectories();
    create_folders->createCityFiles();
    UserInterface ui;

    ui.mainMenu(); 
    

}