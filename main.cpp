#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <climits>
#include <algorithm>
#include <sstream>

#ifdef USE_MYSQL
  #include <mysql_driver.h>
  #include <mysql_connection.h>
  #include <cppconn/statement.h>
  #include <cppconn/resultset.h>
  #include <cppconn/prepared_statement.h>
#endif

using namespace std;

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BLUE    "\033[34m"
#define WHITE   "\033[37m"

double ae     = 0;   // Airborne Eats total
double jc     = 0;   // JetSet Cafe total
double tt     = 0;   // Travel Treats total
double lugg   = 0;   // Luggage charges
double ticket = 0;   // Flight ticket cost
double pay    = 0;   // Grand total payable

int    lugg_weight = 0;
int    mv_choice   = 0;   // which food outlet was visited last

// Flight IDs chosen
int fd = -1;   // domestic flight id  (-1 = none)
int fi = -1;   // international flight id (-1 = none)

// Food item quantity counters removed (unused)

// Outlets visited (mirrors Python mv list: 1=AE, 2=JC, 3=TT)
vector<int> mv;


struct Flight {
    int    id;
    string airline;
    string flightNumber;
    string departureCity;
    string arrivalCity;
    string departureTime;
    string arrivalTime;
};

struct Passenger {
    int    id;
    string name;
    string email;
    string phone;
    string passport;
};

// ── Sample domestic flights (mirrors dom_flights table) ─────
vector<Flight> domFlights = {
    {1,  "SkyVista Airlines",     "SVA1024", "New Delhi",     "Mumbai",        "2024-10-05 08:15", "2024-10-05 10:12"},
    {2,  "AeroWave Airlines",     "AWA2025", "Mumbai",        "New Delhi",     "2024-10-07 09:47", "2024-10-07 11:23"},
    {3,  "StellarAir",            "STA3036", "Chennai",       "Bangalore",     "2024-10-10 10:03", "2024-10-10 12:28"},
    {4,  "GlobalWind Airways",    "GWA4047", "Bangalore",     "Chennai",       "2024-10-12 11:06", "2024-10-12 13:32"},
    {5,  "AirZephyr",             "AZA5058", "Kolkata",       "Hyderabad",     "2024-10-14 12:24", "2024-10-14 14:49"},
    {6,  "CloudHorizon Flights",  "CHF6069", "Hyderabad",     "Kolkata",       "2024-10-16 13:35", "2024-10-16 15:59"},
    {7,  "JetStream Aviation",    "JSA7071", "Pune",          "Goa",           "2024-10-18 14:38", "2024-10-18 16:45"},
    {8,  "NimbusSkyways",         "NSY8082", "Goa",           "Pune",          "2024-10-20 16:02", "2024-10-20 18:27"},
    {9,  "SkyQuest Airlines",     "SQA9093", "Ahmedabad",     "Chandigarh",    "2024-10-22 17:41", "2024-10-22 19:36"},
    {10, "AeroElite",             "AEL1010", "Chandigarh",    "Ahmedabad",     "2024-10-25 18:13", "2024-10-25 20:28"},
    {11, "SkyVista Airlines",     "SVA1111", "Lucknow",       "Surat",         "2024-10-28 08:19", "2024-10-28 10:35"},
    {12, "AeroWave Airlines",     "AWA2122", "Surat",         "Lucknow",       "2024-11-01 09:21", "2024-11-01 11:52"},
    {13, "StellarAir",            "STA3133", "Patna",         "Bhopal",        "2024-11-03 10:14", "2024-11-03 12:07"},
    {14, "GlobalWind Airways",    "GWA4144", "Bhopal",        "Patna",         "2024-11-05 11:22", "2024-11-05 13:03"},
    {15, "AirZephyr",             "AZA5155", "Visakhapatnam", "Vadodara",      "2024-11-07 12:39", "2024-11-07 14:26"},
    {16, "CloudHorizon Flights",  "CHF6166", "Vadodara",      "Visakhapatnam", "2024-11-09 13:53", "2024-11-09 15:17"},
    {17, "JetStream Aviation",    "JSA7177", "Coimbatore",    "Indore",        "2024-11-12 15:47", "2024-11-12 17:10"},
    {18, "NimbusSkyways",         "NSY8188", "Indore",        "Coimbatore",    "2024-11-14 16:05", "2024-11-14 18:21"},
    {19, "SkyQuest Airlines",     "SQA9199", "Nagpur",        "Jaipur",        "2024-11-16 17:18", "2024-11-16 19:35"},
    {20, "AeroElite",             "AEL1020", "Jaipur",        "Nagpur",        "2024-11-18 18:24", "2024-11-18 20:39"},
};

// ── Sample international flights (mirrors int_flights table) ─
vector<Flight> intFlights = {
    {1,  "SkyVistaAirlines",      "SVA1024", "NewDelhi",   "London",    "2024-11-01 08:15", "2024-11-01 16:45"},
    {2,  "SkyVistaAirlines",      "SVA2025", "London",     "New Delhi", "2024-11-02 09:42", "2024-11-02 18:22"},
    {3,  "AeroWaveAirlines",      "AWA3036", "Mumbai",     "NewYork",   "2024-11-03 10:33", "2024-11-03 19:54"},
    {4,  "AeroWaveAirlines",      "AWA4047", "NewYork",    "Mumbai",    "2024-11-04 11:21", "2024-11-05 07:56"},
    {5,  "StellarAir",            "STA5058", "Chennai",    "Singapore", "2024-11-05 12:54", "2024-11-05 18:44"},
    {6,  "StellarAir",            "STA6069", "Singapore",  "Chennai",   "2024-11-06 13:37", "2024-11-06 19:01"},
    {7,  "GlobalWindAirways",     "GWA7071", "NewDelhi",   "Dubai",     "2024-11-07 14:23", "2024-11-07 17:55"},
    {8,  "GlobalWindAirways",     "GWA8082", "Dubai",      "NewDelhi",  "2024-11-08 15:16", "2024-11-08 18:37"},
    {9,  "AirZephyr",             "AZA9093", "Bangalore",  "Paris",     "2024-11-09 16:42", "2024-11-09 21:15"},
    {10, "AirZephyr",             "AZA1010", "Paris",      "Bangalore", "2024-11-10 17:08", "2024-11-11 02:22"},
    {11, "CloudHorizonFlights",   "CHF1111", "Mumbai",     "Sydney",    "2024-11-11 18:51", "2024-11-12 14:33"},
    {12, "CloudHorizonFlights",   "CHF2122", "Sydney",     "Mumbai",    "2024-11-13 19:34", "2024-11-14 15:12"},
    {13, "JetStreamAviation",     "JSA3133", "Goa",        "Bangkok",   "2024-11-14 20:23", "2024-11-15 02:01"},
    {14, "JetStreamAviation",     "JSA4144", "Bangkok",    "Goa",       "2024-11-16 21:45", "2024-11-17 05:32"},
    {15, "NimbusSkyways",         "NSY5155", "NewDelhi",   "Tokyo",     "2024-11-17 22:16", "2024-11-18 06:41"},
    {16, "NimbusSkyways",         "NSY6166", "Tokyo",      "NewDelhi",  "2024-11-19 23:52", "2024-11-20 08:29"},
    {17, "SkyQuestAirlines",      "SQA7177", "Kolkata",    "Toronto",   "2024-11-20 00:14", "2024-11-20 12:55"},
    {18, "SkyQuestAirlines",      "SQA8188", "Toronto",    "Kolkata",   "2024-11-21 01:23", "2024-11-21 13:11"},
    {19, "AeroElite",             "AEL9199", "Hyderabad",  "London",    "2024-11-22 02:35", "2024-11-22 10:29"},
    {20, "AeroElite",             "AEL1020", "London",     "Hyderabad", "2024-11-23 03:44", "2024-11-23 11:57"},
};

// ── Food menus (mirrors DB tables) ──────────────────────────
map<string,pair<string,double>> airborneEatsMenu = {
    {"VB",   {"Vegetable Biryani",           100.00}},
    {"PBMN", {"Paneer Butter Masala w/Naan", 120.00}},
    {"TGCR", {"Thai Green Curry with Rice",  110.00}},
    {"VL",   {"Vegetable Lasagna",           130.00}},
    {"FW",   {"Falafel Wrap",                 90.00}},
    {"MP",   {"Margarita Pizza",             100.00}},
    {"VST",  {"Vegetable Stir-fry w/Tofu",   110.00}},
    {"M",    {"Mojito",                       70.00}},
    {"B",    {"Buttermilk",                   30.00}},
    {"VPC",  {"Virgin Pina Colada",           80.00}},
};

map<string,pair<string,double>> jetsetCafeMenu = {
    {"PS",  {"Paneer Sandwich",        60.00}},
    {"VQ",  {"Vegetable Quiche",       70.00}},
    {"CS",  {"Caprese Salad",          80.00}},
    {"VB",  {"Veggie Burger",          90.00}},
    {"CC",  {"Cheese Croissant",       50.00}},
    {"GS",  {"Greek Salad",            70.00}},
    {"BCC", {"Bagel with Cream Cheese",60.00}},
    {"C",   {"Cappuccino",             60.00}},
    {"S",   {"Smoothie",               80.00}},
    {"HC",  {"Hot Chocolate",          70.00}},
};

map<string,pair<string,double>> travelTreatsMenu = {
    {"S",   {"Samosa",                30.00}},
    {"SR",  {"Spring Rolls",          50.00}},
    {"VP",  {"Veg Puffs",             40.00}},
    {"PT",  {"Paneer Tikka",          80.00}},
    {"B",   {"Bruschetta",            60.00}},
    {"NS",  {"Nachos with Salsa",     70.00}},
    {"HPB", {"Hummus with Pita Bread",90.00}},
    {"ML",  {"Mango Lassi",           50.00}},
    {"LIT", {"Lemon Iced Tea",        40.00}},
    {"MC",  {"Masala Chai",           20.00}},
};

// Luggage pricing (mirrors luggage table)
map<int,int> luggagePrice = {{2,1499},{5,3199},{10,6199},{30,29999}};

//  UTILITY FUNCTIONS
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printLine(char ch = '-', int len = 60) {
    cout << string(len, ch) << "\n";
}

void printHeader(const string& title) {
    printLine('=');
    cout << BOLD << CYAN
         << "  " << title << "\n"
         << RESET;
    printLine('=');
}

void pause() {
    cout << "\n" << YELLOW << "Press ENTER to continue..." << RESET;
    string dummy;
    getline(cin, dummy);
}

int getRandInt(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

// Read a validated integer from stdin
int readInt(const string& prompt, int lo = INT_MIN, int hi = INT_MAX) {
    int val;
    while (true) {
        cout << prompt;
        string line;
        getline(cin, line);
        stringstream ss(line);
        if (ss >> val && val >= lo && val <= hi && ss.eof()) {
            return val;
        }
        cout << RED << "  Invalid input. Please try again.\n" << RESET;
    }
}

string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

//  SHOW ALL FLIGHTS
void showDomFlights() {
    printHeader("AVAILABLE DOMESTIC FLIGHTS");
    cout << left
         << setw(4)  << "ID"
         << setw(25) << "Airline"
         << setw(12) << "Flight No"
         << setw(18) << "Departure City"
         << setw(18) << "Arrival City"
         << setw(22) << "Departure Time"
         << "Arrival Time\n";
    printLine();
    for (auto& f : domFlights) {
        cout << left
             << setw(4)  << f.id
             << setw(25) << f.airline
             << setw(12) << f.flightNumber
             << setw(18) << f.departureCity
             << setw(18) << f.arrivalCity
             << setw(22) << f.departureTime
             << f.arrivalTime << "\n";
    }
}

void showIntFlights() {
    printHeader("AVAILABLE INTERNATIONAL FLIGHTS");
    cout << left
         << setw(4)  << "ID"
         << setw(25) << "Airline"
         << setw(12) << "Flight No"
         << setw(18) << "Departure City"
         << setw(18) << "Arrival City"
         << setw(22) << "Departure Time"
         << "Arrival Time\n";
    printLine();
    for (auto& f : intFlights) {
        cout << left
             << setw(4)  << f.id
             << setw(25) << f.airline
             << setw(12) << f.flightNumber
             << setw(18) << f.departureCity
             << setw(18) << f.arrivalCity
             << setw(22) << f.departureTime
             << f.arrivalTime << "\n";
    }
}

double orderFood(const string& outletName,
                 const map<string,pair<string,double>>& menu)
{
    double subtotal = 0.0;
    map<string,int> quantities;
    for (auto& item : menu) quantities[item.first] = 0;

    printHeader(outletName + " - Menu");
    cout << left << setw(8) << "Code" << setw(35) << "Item" << "Price (Rs)\n";
    printLine();
    for (auto& item : menu) {
        cout << left
             << setw(8)  << item.first
             << setw(35) << item.second.first
             << fixed << setprecision(2) << item.second.second << "\n";
    }
    printLine();

    cout << "\n" << YELLOW
         << "Enter item code to add to order (or 'done' to finish):\n"
         << RESET;

    while (true) {
        string code = readLine("  > ");
        // convert to uppercase
        transform(code.begin(), code.end(), code.begin(), ::toupper);
        if (code == "DONE" || code == "0") break;

        if (menu.count(code)) {
            quantities[code]++;
            subtotal += menu.at(code).second;
            cout << GREEN << "  Added: " << menu.at(code).first
                 << " (x" << quantities[code] << ")\n" << RESET;
        } else {
            cout << RED << "  Invalid code. Try again.\n" << RESET;
        }
    }

    // GST 18%
    double tax = 0.18 * subtotal;
    double total = subtotal + tax;

    cout << "\n";
    printLine('-', 40);
    cout << BOLD << "  Order Summary – " << outletName << "\n" << RESET;
    printLine('-', 40);
    for (auto& kv : quantities) {
        if (kv.second > 0) {
            cout << "  " << kv.second << " x " << menu.at(kv.first).first
                 << "  Rs " << fixed << setprecision(2)
                 << menu.at(kv.first).second * kv.second << "\n";
        }
    }
    cout << "  GST (18%): Rs " << fixed << setprecision(2) << tax << "\n";
    cout << BOLD << "  Payable:   Rs " << total << "\n" << RESET;
    printLine('-', 40);

    if (total > 0) {
        cout << "\nWhere would you like your order?\n"
             << "  1. Food Court\n"
             << "  2. In-Flight\n";
        int loc = readInt("  Choice: ", 1, 2);
        if (loc == 1)
            cout << GREEN << "\n  Order confirmed! Collect from Food Court.\n" << RESET;
        else
            cout << GREEN << "\n  Order confirmed! Will be served in-flight.\n" << RESET;
    }

    return total;
}

void airborne_eats() {
    ae += orderFood("Airborne Eats", airborneEatsMenu);
}

void jetset_cafe() {
    jc += orderFood("JetSet Cafe", jetsetCafeMenu);
}

void travel_treats() {
    tt += orderFood("Travel Treats", travelTreatsMenu);
}

//  MENU VIEW  (mirrors menuview())
void menuview() {
    while (true) {
        printHeader("FOOD COURT");
        cout << "  1. Airborne Eats\n"
             << "  2. JetSet Cafe\n"
             << "  3. Travel Treats\n"
             << "  4. Checkout (done ordering)\n";
        int choice = readInt("  Select outlet: ", 1, 4);
        if (choice == 4) break;
        mv.push_back(choice);
        if      (choice == 1) airborne_eats();
        else if (choice == 2) jetset_cafe();
        else if (choice == 3) travel_treats();
    }
}

void luggage() {
    printHeader("EXTRA LUGGAGE");

    // Show pricing table
    cout << "  Excess weight options and prices:\n\n";
    cout << left << setw(15) << "  Extra (kg)" << "Price (Rs)\n";
    printLine('-', 35);
    for (auto& kv : luggagePrice) {
        cout << "  " << setw(13) << kv.first << kv.second << "\n";
    }
    printLine('-', 35);
    cout << "\n  Select extra luggage weight:\n"
         << "  1.  0 kg  (no extra)\n"
         << "  2.  2 kg  – Rs 1499\n"
         << "  3.  5 kg  – Rs 3199\n"
         << "  4. 10 kg  – Rs 6199\n"
         << "  5. 30 kg  – Rs 29999\n";

    int ch = readInt("  Choice: ", 1, 5);
    switch (ch) {
        case 1: lugg_weight = 0;  lugg = 0;     break;
        case 2: lugg_weight = 2;  lugg = 1499;  break;
        case 3: lugg_weight = 5;  lugg = 3199;  break;
        case 4: lugg_weight = 10; lugg = 6199;  break;
        case 5: lugg_weight = 30; lugg = 29999; break;
    }

    if (lugg > 0) {
        cout << GREEN << "\n  Luggage charge: Rs " << lugg
             << " for " << lugg_weight << " kg extra.\n" << RESET;
    } else {
        cout << GREEN << "\n  No extra luggage selected.\n" << RESET;
    }

    pay += lugg;
}

void dom_price() {
    showDomFlights();
    fd = readInt("\nEnter desired Flight ID (1-20, 0 to cancel): ", 0, 20);
    if (fd == 0) {
        cout << YELLOW << "  Logging out...\n" << RESET;
        fd = -1;
        return;
    }

    // Available seats (random, matching Python logic)
    int f_seats = getRandInt(1,15) * 3;
    int b_seats = getRandInt(1,15) * 4;
    int e_seats = getRandInt(1,15) * 7;

    cout << "\n  Available tickets:\n"
         << "    1. First class   (" << f_seats << ") – Rs 43199 each\n"
         << "    2. Business class(" << b_seats << ") – Rs 23099 each\n"
         << "    3. Economy class (" << e_seats << ") – Rs  7699 each\n"
         << "    4. Exit\n";

    double S = 0;
    while (true) {
        int cls = readInt("  Select class: ", 1, 4);
        if (cls == 4) { cout << YELLOW << "  Logging out...\n" << RESET; fd=-1; return; }

        int maxSeats = (cls==1) ? f_seats : (cls==2) ? b_seats : e_seats;
        double price = (cls==1) ? 43199.0 : (cls==2) ? 23099.0 : 7699.0;

        int n = readInt("  Enter No. of Passengers: ", 1, maxSeats);
        S = price * n;
        break;
    }

    double gst  = 0.18 * S;
    double total = S + gst;
    cout << fixed << setprecision(2);
    cout << GREEN << "\n  Payable amount = Rs " << total << RESET << "\n";

    ticket += total;
    pay    += total;
}

void int_price() {
    showIntFlights();
    fi = readInt("\nEnter desired Flight ID (1-20, 0 to cancel): ", 0, 20);
    if (fi == 0) {
        cout << YELLOW << "  Logging out...\n" << RESET;
        fi = -1;
        return;
    }

    int f_seats = getRandInt(1,15) * 3;
    int b_seats = getRandInt(1,15) * 4;
    int e_seats = getRandInt(1,15) * 7;

    cout << "\n  Available tickets:\n"
         << "    1. First class   (" << f_seats << ") – Rs 334949 each\n"
         << "    2. Business class(" << b_seats << ") – Rs 145989 each\n"
         << "    3. Economy class (" << e_seats << ") – Rs  76099 each\n"
         << "    4. Exit\n";

    double S = 0;
    while (true) {
        int cls = readInt("  Select class: ", 1, 4);
        if (cls == 4) { cout << YELLOW << "  Logging out...\n" << RESET; fi=-1; return; }

        int maxSeats = (cls==1) ? f_seats : (cls==2) ? b_seats : e_seats;
        double price = (cls==1) ? 334949.0 : (cls==2) ? 145989.0 : 76099.0;

        int n = readInt("  Enter No. of Passengers: ", 1, maxSeats);
        S = price * n;
        break;
    }

    double gst  = 0.18 * S;
    double total = S + gst;
    cout << fixed << setprecision(2);
    cout << GREEN << "\n  Payable amount = Rs " << total << RESET << "\n";

    ticket += total;
    pay    += total;
}

void travel() {
    printHeader("TRAVEL TYPE");
    cout << "  1. Domestic\n"
         << "  2. International\n"
         << "  3. Cancel\n";
    int ch = readInt("  Choice: ", 1, 3);
    if      (ch == 1) dom_price();
    else if (ch == 2) int_price();
    else    cout << YELLOW << "  Cancelled.\n" << RESET;
}

Passenger currentPassenger;   // stores logged-in / registered passenger

void registerPassenger() {
    printHeader("PASSENGER REGISTRATION");
    currentPassenger.name     = readLine("  Enter Name          : ");
    currentPassenger.email    = readLine("  Enter Email Address : ");
    currentPassenger.phone    = readLine("  Enter Phone Number  : ");
    currentPassenger.passport = readLine("  Enter Passport No.  : ");
    currentPassenger.id       = 1000 + rand() % 9000;   // simulated auto-increment

    cout << GREEN << "\n  Registration successful! Your Passenger ID: "
         << currentPassenger.id << "\n" << RESET; }

//  CHECKOUT  (mirrors checkout())
void checkout() {
    printHeader("CHECKOUT");

    // Determine which flight was booked
    Flight bookedFlight;
    bool   isDomestic = (fd != -1);

    if (isDomestic && fd >= 1 && fd <= 20) {
        bookedFlight = domFlights[fd - 1];
    } else if (!isDomestic && fi >= 1 && fi <= 20) {
        bookedFlight = intFlights[fi - 1];
    } else {
        cout << RED << "  No flight selected. Cannot checkout.\n" << RESET;
        return;
    }

    // ── Print payment bill ──
    time_t now = time(nullptr);
    char   ts[20];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    cout << "\n";
    printLine('-', 55);
    cout << BOLD << "  BookMyFlight – Payment Bill" << RESET << "\n";
    printLine('-', 55);
    cout << "  Passenger  : " << currentPassenger.name  << "\n"
         << "  Passport   : " << currentPassenger.passport << "\n"
         << "  Phone      : " << currentPassenger.phone << "\n\n"
         << "  Airline    : " << bookedFlight.airline       << "\n"
         << "  Flight No  : " << bookedFlight.flightNumber  << "\n"
         << "  From       : " << bookedFlight.departureCity << "\n"
         << "  To         : " << bookedFlight.arrivalCity   << "\n"
         << "  Departure  : " << bookedFlight.departureTime << "\n"
         << "  Arrival    : " << bookedFlight.arrivalTime   << "\n";
    printLine('-', 55);
    cout << fixed << setprecision(2)
         << "  Airborne Eats    : Rs " << ae     << "\n"
         << "  JetSet Cafe      : Rs " << jc     << "\n"
         << "  Travel Treats    : Rs " << tt     << "\n"
         << "  Extra Luggage    : Rs " << lugg   << "\n"
         << "  Flight Tickets   : Rs " << ticket << "\n";
    printLine('-', 55);
    cout << BOLD << "  TOTAL PAYABLE    : Rs " << pay << "\n" << RESET;
    printLine('-', 55);
    cout << "  Date/Time  : " << ts << "\n";
    printLine('-', 55);

    // ── Payment mode ──
    cout << "\n  Select payment mode:\n"
         << "  1. Cash (PayCounter near Terminal 3)\n"
         << "  2. UPI\n"
         << "  3. Cancel Payment\n";
    int pm = readInt("  Choice: ", 1, 3);

    if (pm == 1) {
        cout << GREEN << "\n  Checkout complete! Please pay at PayCounter near Terminal 3.\n"
             << RESET;
    } else if (pm == 2) {
        string upi = readLine("  Enter UPI ID: ");
        cout << YELLOW << "\n  Pending UPI payment...\n" << RESET;
        // simulate processing
        cout << GREEN << "  Payment successful through UPI!\n" << RESET;
    } else {
        // Cancel: mirrors Python cancel logic
        cout << "\n  Are you sure you want to cancel?\n"
             << "  1. Yes – cancel booking\n"
             << "  2. No  – take me back\n";
        int conf = readInt("  Choice: ", 1, 2);
        if (conf == 1) {
            cout << RED << "\n  Reservation cancelled. All records removed.\n" << RESET;
            // In MySQL: DELETE FROM dom/int_reservations ORDER BY ... LIMIT 1
        } else {
            // Non-recursive: payment loop handled above
        }
    }
}

//  FINAL CONFIRMATION  (mirrors final())
void final_confirmation() {
    printHeader("BOOKING CONFIRMED");
    cout << BOLD << GREEN
         << "\n  Thank you for choosing BookMyFlight!\n"
         << "  Your Ticket to the Skies is confirmed.\n"
         << RESET << "\n";

    cout << "  Would you like to leave feedback? (1=Yes / 2=Later): ";
    int fb = readInt("", 1, 2);
    if (fb == 1) {
        string feedback = readLine("\n  Your feedback: ");
        cout << GREEN << "\n  Thank you for the feedback! It helps us improve our services.\n"
             << RESET;
    }
}

//  RESET GLOBALS (for fresh session)
void resetGlobals() {
    ae = jc = tt = lugg = ticket = pay = 0.0;
    lugg_weight = 0;
    fd = fi = -1;
    mv.clear();
    currentPassenger = Passenger();
}

//  LOGIN / SIGN-UP  (mirrors the main while-True loop)
// Hardcoded demo credentials (Python used b='test', c='wow')
const string DEMO_USER = "test";
const string DEMO_PASS = "wow";

string currentUser = DEMO_USER;
string currentPass = DEMO_PASS;

bool doLogin() {
    string user = readLine("  Username: ");
    string pass = readLine("  Password: ");
    return (user == currentUser && pass == currentPass);
}

void doSignUp() {
    printHeader("SIGN UP");
    currentUser = readLine("  Choose username: ");
    currentPass = readLine("  Choose password: ");
    cout << GREEN << "\n  Account created! Please log in.\n" << RESET;
}

//  MAIN ENTRY POINT
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    while (true) {
        clearScreen();
        printHeader("Welcome to BookMyFlight!");
        cout << "  1. Login\n"
             << "  2. Sign Up\n"
             << "  3. Exit\n";
        int choice = readInt("  Choice: ", 1, 3);

        // ── EXIT ──
        if (choice == 3) {
            cout << CYAN << "\n  Goodbye!\n" << RESET;
            break;
        }

        // ── SIGN UP ──
        if (choice == 2) {
            doSignUp();
            continue;
        }

        // ── LOGIN ──
        if (!doLogin()) {
            cout << RED << "\n  Incorrect credentials. Try again.\n" << RESET;
            pause();
            continue;
        }

        cout << GREEN << "\n  User Login Successful!\n" << RESET;
        resetGlobals();

        // ── Main booking flow (matches Python sequence) ──────
        registerPassenger();   // collect passenger details
        travel();              // choose + book flight
        menuview();            // order food (optional)
        luggage();             // extra luggage (optional)
        checkout();            // billing + payment
        final_confirmation();  // thank-you + feedback

        cout << "\n";
        printLine('=');
        cout << "  Session complete. Returning to main menu...\n";
        printLine('=');
        pause();
    }

    return 0;
}
