#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC

using namespace std;

class Dealer;

class Car {
    friend ostream& operator<<(ostream& out, const Car& car);
public:
    Car(shared_ptr<Dealer> d, const char* m = "Ford", const char* l = nullptr, int ml = 0) : dealer(d), model(m),
    license(l ? l : (rand() % 2 ? "FOO-" : "BAR-") + to_string(rand() % 999 + 1)), mileage(ml ? ml : rand() % 10000) {}
    ~Car() { cout << model << " " << license << " deleted" << endl; };
    void Read();
    string GetLicense() const { return license; }
private:
    string model;
    string license;
    int mileage;
    weak_ptr<Dealer> dealer;
};

void Car::Read()
{
    model = "Seat";
    license = (rand() % 2 ? "ZAP-" : "ZIP-") + to_string(rand() % 999 + 1);
    mileage = 10000 + rand() % 10000;
}
/*
 * class Website
 * run() - runs a loop that sleeps for 10 s (change to 20 s)
 * prints a list of advertised cars
 * loop and thread - stop if the list of cars is empty
 */
class Website {
public:
    Website(const char* n = nullptr) : name(n ? n : "www.cars" + to_string(rand() % 99 + 1) + ".com") {}
    ~Website() { cout << name << " deleted" << endl; };
    void advertise(shared_ptr<Car> car) { listing.push_back(car); }
    void print(ostream& out = cout) {
        out << name << endl;
        for (auto car : listing) {
            out << *car; out << name << " end of list" << endl;
        }

    }
    void clearListings() { listing.clear(); }
    void run() {
        while (true) {
            this_thread::sleep_for(chrono::seconds(20));
            if (listing.empty()) {
                terminate();
                break;
            }
            print();
        }
    }
private:
    vector<shared_ptr<Car>> listing;
    string name;
};

class Dealer : public enable_shared_from_this<Dealer> {
    friend ostream& operator<<(ostream& out, const Dealer& dealer);
public:
    Dealer(const char* name_ = "John Doe") : name(name_) {};
    ~Dealer() { cout << name << " deleted" << endl; };
    void buy();
    void sell();
    void add(shared_ptr<Car> car) {
        cars.push_back(car);
        for (auto site : sites)
            site->advertise(car);
    }
    void add_site(shared_ptr<Website> w) { sites.push_back(w); }
    const string& get_name() const { return name; }

private:
    string name;
    vector<shared_ptr<Car>> cars;
    vector<shared_ptr<Website>> sites;
};

void Dealer::buy()
{
    auto car = make_shared<Car>(shared_from_this());
    car->Read();
    add(car);
}

void Dealer::sell()
{
    cout << *this;
    cout << "Enter license of car you want to buy" << endl;
    string license;
    cin >> license;
    auto ci = find_if(cars.begin(), cars.end(), [&license](auto c) {return license == c->GetLicense(); });
    if (ci != cars.end()) {
        cout << "You bought the car." << endl;
        cars.erase(ci);
    }
}

ostream& operator<<(ostream& out, const Car& car)
{
    auto d = car.dealer.lock();
    auto dealer = d ? d->get_name() : "Dealer not found";
    cout << "\nDealer: " << dealer << " --- " << "Model: " << car.model << " --- " << "License: " <<
    car.license << " --- " << "Mileage: " << car.mileage << endl;

    return out;
}

ostream& operator<<(ostream& out, const Dealer& dealer)
{
    cout << dealer.name << "'s cars for sale" << endl;
    for (auto car : dealer.cars) cout << *car;
    cout << "End of " << dealer.name << "'s cars listing" << endl;

    return out;
}

/*
 * car_sales()
 * asks user to select dealer to buy a car --> sell()
 * or to exit program
 */
void car_sales()
{

    cout << "Car sales started" << endl;
    auto wa = make_shared<Website>("www.autos.com");
    auto wb = make_shared<Website>("www.bilar.com");
    auto wc = make_shared<Website>("www.cars.com");
    auto a = make_shared<Dealer>("Alan Aldis");
    auto b = make_shared<Dealer>("Bill Munny");

    {
        auto c = make_shared<Dealer>("Casey Ball");
        auto ca = make_shared<Car>(c);
        auto cb = make_shared<Car>(c);

        a->add_site(wa);
        a->add_site(wb);
        b->add_site(wb);
        b->add_site(wc);
        c->add_site(wa);
        c->add_site(wb);
        c->add_site(wc);

        a->buy();
        a->buy();
        a->buy();
        a->buy();

        b->buy();
        b->buy();
        b->buy();

        c->buy();
        c->buy();
        c->add(ca);
        c->add(cb);

        auto t1 = make_unique<thread>(&Website::run, wa);
        auto t2 = make_unique<thread>(&Website::run, wb);
        auto t3 = make_unique<thread>(&Website::run, wc);

        while (true) {
            cout << "Enter website to buy from\n1) Alan 2) Bill 3) Casey" << endl << "Other Exit" << endl;

            int choice;
            cin >> choice;
            if (choice == 1)
                a->sell();
            else if (choice == 2)
                b->sell();
            else if (choice == 3)
                c->sell();
            else {
                wa->clearListings();
                wb->clearListings();
                wc->clearListings();
                t1->join();
                t2->join();
                t3->join();
                break;
            }
        }
    }

    cout << "Car sales ended" << endl;
}

int main(int argc, char** argv) {
    _CrtMemState s1;
    _CrtMemCheckpoint(&s1);

    srand(time(NULL));

    car_sales();

    _CrtMemState s2, s3;
    _CrtMemCheckpoint(&s2);
    if (_CrtMemDifference(&s3, &s1, &s2)) {
        _CrtDumpMemoryLeaks();
        _CrtMemDumpStatistics(&s3);
    }

    return 0;
}

