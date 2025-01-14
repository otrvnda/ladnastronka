#include <iostream>
#include <string>
#include <vector>
#include "Klasy.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "date.hpp"
#include <cstdlib>

using namespace std;




//Ksia¿ki
Ksiazka::Ksiazka() : id(0), ISBN(0), tytul(""), autor(""), okladka("") {}

Ksiazka::Ksiazka(string t, string au, int isbn, string okl)
	: tytul(std::move(t)), autor(std::move(au)), ISBN(isbn), okladka(std::move(okl)) {}


std::ostream& operator<<(std::ostream& os, const Ksiazka& k) {
	os << "[" << k.id << ", " << k.tytul << ", " << k.autor << ", " << k.ISBN << "]\n";
	return os;
}




//Baza Ksi¹¿ek

BazaKsiazek::BazaKsiazek() {
	ifstream plik(nazwa_pliku);
	if (!plik) {
		ofstream nowy_plik(nazwa_pliku); // Tworzymy plik, jeœli nie istnieje
	}
	else {
		loadFromFile();
	}
}

void BazaKsiazek::loadFromFile() {
	lista_ksiazek.clear();

	ifstream plik(nazwa_pliku);
	if (!plik) {
		std::cerr << "Nie mo¿na otworzyæ pliku: " << nazwa_pliku << std::endl;
		return;
	}

	string linia;
	while (std::getline(plik, linia)) {
		std::stringstream ss(linia);
		string id_str, tytul, autor, isbn_str, okladka;

		// Rozdziel linie na poszczególne czêœci, oddzielone przecinkami
		if (std::getline(ss, id_str, ',') &&
			std::getline(ss, tytul, ',') &&
			std::getline(ss, autor, ',') &&
			std::getline(ss, isbn_str, ',') &&
			std::getline(ss, okladka)) {

			// Usuñ ewentualne spacje na pocz¹tku i koñcu
			tytul = trim(tytul);
			autor = trim(autor);
			okladka = trim(okladka);

			int id = std::stoi(trim(id_str));
			int isbn = std::stoi(trim(isbn_str));

			// Utwórz obiekt Ksiazka i dodaj go do wektora
			Ksiazka ksiazka(tytul, autor, isbn, okladka);
			ksiazka.id = id;
			lista_ksiazek.push_back(ksiazka);
		}
	}

	plik.close();
}

void BazaKsiazek::saveToFile() {
	ofstream plik(nazwa_pliku, ios::trunc);
	if (!plik) {
		std::cerr << "Nie mo¿na otworzyæ pliku do zapisu: " << nazwa_pliku << std::endl;
		return;
	}

	for (const auto& ksiazka : lista_ksiazek) {
		plik << ksiazka.id << ", " << ksiazka.tytul << ", " << ksiazka.autor << ", "
			<< ksiazka.ISBN << ", " << ksiazka.okladka << "\n";
	}

	plik.close();
}

// Funkcja pomocnicza do usuwania spacji z pocz¹tku i koñca stringa
static std::string trim(const std::string& str) {
	const auto strBegin = str.find_first_not_of(" \t");
	const auto strEnd = str.find_last_not_of(" \t");
	const auto strRange = strEnd - strBegin + 1;

	return strBegin == std::string::npos ? "" : str.substr(strBegin, strRange);
}

void BazaKsiazek::add(string t, string au, int isbn, string okl) {
	if (wyszukaj_czy_jest(isbn)) {
		throw runtime_error("Ksi¹¿ka o podanym ISBN ju¿ istnieje.");
	}
	else {
		if (wyszukaj_czy_jest(t)) {
			throw runtime_error("Ksi¹¿ka o podanym tytule ju¿ istnieje.");
		}
		else {
			Ksiazka nowa(t, au, isbn, okl);
			nowa.id = int(lista_ksiazek.size() + 1);
			lista_ksiazek.push_back(nowa);

			// Dodanie do pliku
			ofstream plik(nazwa_pliku, ios::app);
			plik << nowa.id << ", " << nowa.tytul << ", " << nowa.autor << ", "
				<< nowa.ISBN << ", " << nowa.okladka << "\n";
			plik.close();

			this->loadFromFile(); // Aktualizacja listy ksi¹¿ek
		}
	}
}
void BazaKsiazek::usun_ksiazke(int isbn) {
	auto it = std::find_if(lista_ksiazek.begin(), lista_ksiazek.end(), [&](const Ksiazka& k) {
		return k.ISBN == isbn;
		});

	if (it != lista_ksiazek.end()) {
		lista_ksiazek.erase(it);
		saveToFile();
	}
}


const vector<Ksiazka>& BazaKsiazek::getBooks() const {
	return lista_ksiazek;
}

void BazaKsiazek::setBooks() {
	loadFromFile();
}

bool BazaKsiazek::wyszukaj_czy_jest(int isbn) const {
	for (const auto& ksiazka : lista_ksiazek) {
		if (ksiazka.ISBN == isbn) {
			return true;
		}
	}
	return false;
}

bool BazaKsiazek::wyszukaj_czy_jest(string tyt) const {
	for (const auto& ksiazka : lista_ksiazek) {
		if (ksiazka.tytul == tyt) {
			return true;
		}
	}
	return false;
}

Ksiazka BazaKsiazek::wyszukaj(const std::string& tyt) const {
	for (const auto& ksiazka : lista_ksiazek) {
		if (ksiazka.tytul == tyt) {
			return ksiazka;
		}
	}
	throw std::runtime_error("Nie znaleziono ksi¹¿ki o podanym tytule.");
}

Ksiazka BazaKsiazek::wyszukaj(int isbn) const {
	for (const auto& ksiazka : lista_ksiazek) {
		if (ksiazka.ISBN == isbn) {
			return ksiazka;
		}
	}
	throw runtime_error("Nie znaleziono ksi¹¿ki o podanym ISBN.");
}





//Bibliotekarz

Bibliotekarz::Bibliotekarz() {}

void Bibliotekarz::dodanie_ksiazki(string t, string au, int isbn, string okl, BazaKsiazek baza) {
	baza.add(t, au, isbn, okl);
}


void Bibliotekarz::usuniecie_ksiazki(Ksiazka k, BazaKsiazek baza) {
	int isbn = k.ISBN;
	baza.usun_ksiazke(isbn);
}

void Bibliotekarz::akceptacja_wyp(BazaWypozyczen& baza_wyp, int isbn, BazaSkrytek& baza_skrytek, BazaUzytkownikow& baza_uz, BazaKsiazek& baza_ks) {
	vector<Wypozyczenie> wypozyczenia = baza_wyp.getlista_wypozyczen_pocz();

	auto it = std::find_if(wypozyczenia.begin(), wypozyczenia.end(),
		[isbn](const Wypozyczenie& w) { return w.isbn_ksiazki == isbn; });

	if (it != wypozyczenia.end()) {
		string data_wyp = getCurrentDateTime();
		string data_odd = addDaysToDate(30);

		int numer_skrytki = baza_skrytek.getFirstFree();
		if (numer_skrytki == -1) {
			throw std::runtime_error("Brak wolnych skrytek.");
		}

		// Zajmij skrytkê
		baza_skrytek.zajmij(numer_skrytki, isbn);

		// Dodaj wypo¿yczenie do listy
		baza_wyp.add(data_wyp, data_odd, numer_skrytki, isbn, it->getLogin());

		// Usuñ wypo¿yczenie z poczekalni
		baza_wyp.end_wyp(isbn);

		// Wys³anie emaila
		string email = baza_uz.wyszukaj(it->getLogin()).getEmail();
		string tyt = baza_ks.wyszukaj(isbn).tytul;
		sendEmail(email, it->getLogin(), tyt, data_odd, numer_skrytki);


		std::cout << "Wypo¿yczenie zaakceptowane: ISBN " << isbn << " w skrytce " << numer_skrytki << "." << std::endl;
	}
	else {
		throw std::runtime_error("Nie znaleziono wypo¿yczenia o podanym ISBN.");
	}
}

void Bibliotekarz::sendEmail(string email, string login, string tyt, string dat, int num) {
	//email, login, tytul_ksiazki, data_oddania, numer_skrytki
	ofstream plik("email.txt", ios::trunc);
	if (plik) {
		plik << email<<"/"<<login<<"/"<<tyt<<"/"<<dat<<"/"<<num<<"\n";
		plik.close();


		int wynik = std::system("send_email.exe");

		std::cerr << "\nWynik system(): " << wynik << "\n";
	}
	else {
		cout<<"Nie uda³o siê wys³aæ emaila"<<endl;
	}
}








//Dane osobowe
DaneOsobowe::DaneOsobowe() : numer_tel(0) {}
DaneOsobowe::DaneOsobowe(string i, string n, string e, string a, int tel){

	imie = i;
	nazwisko = n;
	email = e;
	adres = a;
	numer_tel = tel;

}





//Czytelnik
Czytelnik::Czytelnik(string i, string n, string e, string a, int tel, string l, string h){
	//DaneOsobowe Dane_osobowe(i, n, e, a, tel);
	imie = i;
	nazwisko = n;
	email = e;
	adres = a;
	numer_tel = tel;
	login = l;
	haslo = h;
}

Czytelnik::Czytelnik(){
	//DaneOsobowe dane_osobowe;
	imie = "";
	nazwisko = "";
	email = "";
	adres = "";
	numer_tel = 0;
	login = "";
	haslo = "";
}

bool Czytelnik::checkPass(const string& pass) const {
	return haslo == pass;
}


void Czytelnik::wypozycz(int isbn, string login, BazaKsiazek baza_ksiazek, BazaWypozyczen baza_wyp, BazaSkrytek baza_skrytek) {


	if (baza_ksiazek.wyszukaj_czy_jest(isbn)) {
		baza_wyp.przyznanieWyp(login, isbn, baza_skrytek);
	}
	else {
		throw runtime_error("Nie znaleziono ksi¹¿ki o podanym ISBN.");
	}
	
}

string Czytelnik::getImie() {
	return this->imie;
}

string Czytelnik::getNazwisko() {
	return this->nazwisko;
}

string Czytelnik::getEmail() {
	return this->email;
}

string Czytelnik::getAdres() {
	return this->adres;
}

int Czytelnik::getNum() {
	return this->numer_tel;
}

string Czytelnik::getLogin() {
	string Login = this->login;
	return Login;
}

int Czytelnik::getID() {
	return this->id;
}

bool Czytelnik::invertBlodaka() {
	if (this->blokada == true) {
		this->blokada=false;
		return false;
	}
	else {
		this->blokada=true;
		return true;
	}
}


//Baza u¿ytkowników
BazaUzytkownikow::BazaUzytkownikow() {
	ifstream plik(nazwa_pliku);
	ifstream plik_blokady(nazwa_pliku_blokady);
	if (!plik) {
		ofstream nowy_plik(nazwa_pliku); // Tworzymy plik, jeœli nie istnieje
	}
	else {
		loadFromFile();
	}

	if (!plik_blokady) {
		ofstream nowy_plik_blokady(nazwa_pliku_blokady);
	}
	else {
		loadBlockedFromFile();
	}
}

void BazaUzytkownikow::add(string i, string n, string e, string a, int tel, string l, string h) {
	Czytelnik nowy(i, n, e, a, tel, l, h);
	nowy.id = int(lista_czytelnikow.size() + 1);
	lista_czytelnikow.push_back(nowy);

	ofstream plik(nazwa_pliku, ios::app);
	if (!plik) {
		cerr << "B³¹d przy otwieraniu pliku: " << nazwa_pliku << endl;
	}
	else {
		plik << nowy.id << " " << nowy.imie << " " << nowy.nazwisko
			<< " " << nowy.email << " " << nowy.adres
			<< " " << nowy.numer_tel << " " << nowy.login << " " << nowy.haslo << "\n";
		plik.close();
	}

	cout << "Dodano u¿ytkownika: " << nowy.imie << " " << nowy.nazwisko << endl;

	loadFromFile(); // Aktualizacja danych w pamiêci
}

void BazaUzytkownikow::usun(const std::string& login) {
	auto it = std::remove_if(lista_czytelnikow.begin(), lista_czytelnikow.end(),
		[&login](Czytelnik& u) { return u.getLogin() == login; });
	if (it != lista_czytelnikow.end()) {
		lista_czytelnikow.erase(it, lista_czytelnikow.end());
		saveToFile();
	}
	else {
		throw std::runtime_error("Nie znaleziono u¿ytkownika");
	}
}

vector<Czytelnik> BazaUzytkownikow::getlista_czytelnikow() {
	return lista_czytelnikow;
}

void BazaUzytkownikow::loadFromFile() {
	lista_czytelnikow.clear();

	ifstream plik(nazwa_pliku);
	if (!plik) {
		cerr << "B³¹d przy otwieraniu pliku: " << nazwa_pliku << endl;
		return;
	}

	string linia;
	while (getline(plik, linia)) {
		stringstream ss(linia);
		string id_str, imie, nazwisko, email, adres, numer_tel_str, login, haslo;

		if (ss >> id_str >> imie >> nazwisko >> email >> ws) {
			getline(ss, adres, ' ');
			ss >> numer_tel_str >> login >> haslo;

			int id = stoi(id_str);
			int numer_tel = stoi(numer_tel_str);

			Czytelnik czytelnik(imie, nazwisko, email, adres, numer_tel, login, haslo);
			czytelnik.id = id;
			lista_czytelnikow.push_back(czytelnik);
		}
	}

	plik.close();
}


void BazaUzytkownikow::saveToFile() {

	ofstream plik(nazwa_pliku, ios::trunc);
	for (Czytelnik czytelnik : lista_czytelnikow) {
		plik << czytelnik.id << " " << czytelnik.imie << " " << czytelnik.nazwisko << " "
			<< czytelnik.email << " " << czytelnik.adres << " " << czytelnik.numer_tel << " "
			<< czytelnik.login << " " << czytelnik.haslo << "\n";
	}
	plik.close();
}

void BazaUzytkownikow::setUsers() {
	loadFromFile();
}

bool BazaUzytkownikow::wyszukaj_czy_jest(const string& login) const {
	for (const auto& czytelnik : lista_czytelnikow) {
		if (czytelnik.login == login) {
			return true;
		}
	}
	return false;
}

bool BazaUzytkownikow::wyszukaj_czy_jest_zabl(const string& login){
	loadBlockedFromFile();
	for (auto czytelnik : lista_zablokowanych) {
		if (czytelnik.login == login) {
			return true;
		}
	}
	return false;
}

Czytelnik BazaUzytkownikow::wyszukaj(const string& login) const {
	for (const auto& czytelnik : lista_czytelnikow) {
		if (czytelnik.login == login) {
			return czytelnik;
		}
	}

	throw runtime_error("Nie znaleziono u¿ytkownika o podanym loginie.");
}

void BazaUzytkownikow::loadBlockedFromFile() {
	lista_zablokowanych.clear();
	ifstream plik(nazwa_pliku_blokady);
	if (!plik) return;

	string linia;
	while (getline(plik, linia)) {
		stringstream ss(linia);
		string id_str, imie, nazwisko, email, adres, numer_tel_str, login, haslo;

		if (ss >> id_str >> imie >> nazwisko >> email >> ws) {
			getline(ss, adres, ' ');
			ss >> numer_tel_str >> login >> haslo;

			int id = stoi(id_str);
			int numer_tel = stoi(numer_tel_str);

			Czytelnik czytelnik(imie, nazwisko, email, adres, numer_tel, login, haslo);
			czytelnik.id = id;
			lista_zablokowanych.push_back(czytelnik);
		}
	}
	plik.close();
}

void BazaUzytkownikow::saveBlockedToFile() {
	ofstream plik(nazwa_pliku_blokady, ios::trunc);
	if (!plik) return;

	for (const auto& czytelnik : lista_zablokowanych) {
		plik << czytelnik.id << " " << czytelnik.imie << " " << czytelnik.nazwisko << " "
			<< czytelnik.email << " " << czytelnik.adres << " " << czytelnik.numer_tel << " "
			<< czytelnik.login << " " << czytelnik.haslo << "\n";
	}
	plik.close();
}

void BazaUzytkownikow::blockUser(const string& login) {
	auto it = find_if(lista_czytelnikow.begin(), lista_czytelnikow.end(), [&](const Czytelnik& c) {
		return c.login == login;
		});
	if (it != lista_czytelnikow.end()) {
		lista_zablokowanych.push_back(*it);
		cout<<"elolo"<<endl;
		lista_czytelnikow.erase(it);
		saveToFile();
		saveBlockedToFile();
	}
}

void BazaUzytkownikow::unblockUser(const string& login) {
	auto it = find_if(lista_zablokowanych.begin(), lista_zablokowanych.end(), [&](const Czytelnik& c) {
		return c.login == login;
		});
	if (it != lista_zablokowanych.end()) {
		lista_czytelnikow.push_back(*it);
		lista_zablokowanych.erase(it);
		saveToFile();
		saveBlockedToFile();
	}
}

vector<Czytelnik>  BazaUzytkownikow::getlista_zablokowanych() {
	return lista_zablokowanych;
}





//Wypo¿yczenie
Wypozyczenie::Wypozyczenie(string dat_wyp, string dat_od, int skr, int num_ks, string log) {
	data_wypozyczenia = dat_wyp;
	data_oddania = dat_od;
	numer_skrytki = skr;
	isbn_ksiazki = num_ks;
	login_czytelnika = log;
}

ostream& operator<<(ostream& os, const Wypozyczenie& w) {
	os << "Data wypo¿yczenia: " << w.data_wypozyczenia << "\nData oddania: " << w.data_oddania << "\nNumer skrytki: " << w.numer_skrytki << "\nISBN ksi¹¿ki: " << w.isbn_ksiazki << "\nLogin u¿ytkownika: "<<w.login_czytelnika<<"\n";
	return os;
}

string Wypozyczenie::getLogin() {
	return this->login_czytelnika;
}

int Wypozyczenie::getISBN() {
	return this->isbn_ksiazki;
}

void Wypozyczenie::prolong(int num, string data) {
	data_oddania = replaceDay(num, data);
	data_oddania += "*";
}



//Skrytka
Skrytka::Skrytka() : id(0), wolna(true), numer_wypozyczenia(-1) {}

Skrytka::Skrytka(int id, bool wolna, int numer_wypozyczenia)
	: id(id), wolna(wolna), numer_wypozyczenia(numer_wypozyczenia) {}

void Skrytka::setID(int id) {
	this->id = id;
}

int Skrytka::getID() {
	return this->id;
}

int Skrytka::getNumerWypozyczenia() {
	return this->numer_wypozyczenia;
}


//Baza skrytek
BazaSkrytek::BazaSkrytek(int num) {
	ifstream plik(nazwa_pliku);
	if (!plik.is_open() || plik.peek() == std::ifstream::traits_type::eof()) {
		// Jeœli plik nie istnieje lub jest pusty, tworzymy now¹ listê skrytek
		lista_skrytek.clear(); // Upewniamy siê, ¿e lista jest pusta
		for (int i = 0; i < num; i++) {
			int id = i + 1;
			bool wolna = true;
			int numer_wypozyczenia = -1;
			Skrytka nowa(id, wolna, numer_wypozyczenia);
			lista_skrytek.push_back(nowa);
		}
		saveToFile(); // Zapisujemy nowo utworzon¹ listê do pliku
	}
	else {
		loadFromFile(); // Wczytujemy dane z istniej¹cego pliku
		plik.close();
	}
}

void BazaSkrytek::loadFromFile() {
	lista_skrytek.clear();

	ifstream plik(nazwa_pliku);
	if (!plik) return;

	int id, numer_wypozyczenia;
	bool wolna;

	while (plik >> id >> wolna >> numer_wypozyczenia) {
		Skrytka skrytka(id, wolna, numer_wypozyczenia);
		lista_skrytek.push_back(skrytka);
	}
	plik.close();
}

void BazaSkrytek::saveToFile() {
	ofstream plik(nazwa_pliku, ios::trunc);
	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku do zapisu: " << nazwa_pliku << std::endl;
		return;
	}

	for (const auto& skrytka : lista_skrytek) {
		plik << skrytka.id << " " << skrytka.wolna << " " << skrytka.numer_wypozyczenia << "\n";
	}
	plik.close();
}



void BazaSkrytek::setSkrytki() {
	loadFromFile();
}

void BazaSkrytek::add() {
	int id = lista_skrytek.size() + 1;
	int numer_wypozyczenia = -1;
	bool wolna = true;

	Skrytka nowa(id, wolna, numer_wypozyczenia);
	lista_skrytek.push_back(nowa);

	saveToFile(); // Zapisujemy aktualizowan¹ listê do pliku
}


void BazaSkrytek::zajmij(int id, int isbn) {
	if (id > 0 && id <= static_cast<int>(lista_skrytek.size())) {
		Skrytka& skrytka = lista_skrytek[id - 1]; // Indeksy zaczynaj¹ siê od 0

		if (!skrytka.wolna) {
			std::cerr << "Skrytka " << id << " jest ju¿ zajêta!" << std::endl;
			return;
		}

		skrytka.wolna = false;
		skrytka.numer_wypozyczenia = isbn;

		saveToFile(); // Zapisz zmiany do pliku
		std::cout << "Skrytka " << id << " zosta³a zajêta dla ISBN " << isbn << "." << std::endl;
	}
	else {
		std::cerr << "Nieprawid³owy numer skrytki: " << id << std::endl;
	}
}

void BazaSkrytek::zwolnij(int id) {
	if (id > 0 && id <= static_cast<int>(lista_skrytek.size())) {
		Skrytka& skrytka = lista_skrytek[id - 1]; // Indeksy zaczynaj¹ siê od 0
		if (!skrytka.wolna) {
			skrytka.wolna = true;
			skrytka.numer_wypozyczenia = -1; // Ustaw wartoœæ domyœln¹
			saveToFile(); // Zapisz zmiany do pliku
			std::cout << "Skrytka " << id << " zosta³a zwolniona.\n";
		}
		else {
			std::cout << "Skrytka " << id << " ju¿ jest wolna.\n";
		}
	}
	else {
		std::cerr << "Nieprawid³owy numer skrytki: " << id << "\n";
	}
}

bool BazaSkrytek::wyszukaj_czy_jest(int id) const {
	for (const auto& skrytka : lista_skrytek) {
		if (skrytka.id == id) {
			return true;
		}
	}
	return false;
}

Skrytka BazaSkrytek::wyszukaj(int id) const {
	for (const auto& skrytka : lista_skrytek) {
		if (skrytka.id == id) {
			return skrytka;
		}
	}
	throw runtime_error("Nie znaleziono skrytki o podanym ID.");
}

void BazaSkrytek::usun_skrytke(int id) {
	for (size_t i = 0; i < lista_skrytek.size(); ++i) {
		if (lista_skrytek[i].id == id) {
			lista_skrytek.erase(lista_skrytek.begin() + i);
			saveToFile(); // Aktualizacja pliku
			return;
		}
	}
	throw runtime_error("Nie znaleziono skrytki o podanym ID do usuniêcia.");
}

int BazaSkrytek::getFirstFree() {
	for (size_t i = 0; i < lista_skrytek.size(); ++i) {
		if (lista_skrytek[i].wolna) {
			return lista_skrytek[i].id; // Zwróæ ID skrytki, a nie indeks
		}
	}
	return -1; // Brak wolnych skrytek
}


ostream& operator<<(ostream& os, const Skrytka& skrytka) {
	os << "[" << skrytka.id << ", wolna: " << (skrytka.wolna ? "tak" : "nie")
		<< ", numer wypo¿yczenia: " << skrytka.numer_wypozyczenia << "]\n";
	return os;
}





//Baza wypo¿yczeñ
BazaWypozyczen::BazaWypozyczen() {
	ifstream plik(nazwa_pliku);
	ifstream plik_pocz(nazwa_pliku_pocz);
	ifstream plik_zwrotow(nazwa_pliku_zwrotow);
	if (!plik) {
		ofstream nowy_plik(nazwa_pliku); // Tworzymy plik, jeœli nie istnieje
	}
	else {
		loadFromFile();
	}

	if (!plik_pocz) {
		ofstream nowy_plik_pocz(nazwa_pliku_pocz);
	}
	else {
		loadFromFile_pocz();
	}

	if (!plik_zwrotow) {
		ofstream nowy_plik_zwrotow(nazwa_pliku_zwrotow);
	}
	else {
		loadFromFileZwroty();
	}
}

void BazaWypozyczen::loadFromFile() {
	lista_wypozyczen.clear();
	std::ifstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku: " << nazwa_pliku << std::endl;
		return;
	}

	std::string data_wyp, data_od, log;
	int skr, num_ks;
	while (plik >> data_wyp >> data_od >> skr >> num_ks >> log) {
		Wypozyczenie wypozyczenie(data_wyp, data_od, skr, num_ks, log);
		lista_wypozyczen.push_back(wypozyczenie);
	}
	plik.close();
}

void BazaWypozyczen::loadFromFile_pocz() {
	lista_wypozyczen_pocz.clear();
	std::ifstream plik(nazwa_pliku_pocz);

	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku: " << nazwa_pliku_pocz << std::endl;
		return;
	}

	std::string linia;
	while (std::getline(plik, linia)) {
		std::istringstream iss(linia);
		int isbn;
		std::string login, marker;

		if (iss >> isbn >> login) {
			
			lista_wypozyczen_pocz.emplace_back("", "", -1, isbn, login);

		}
	}

	plik.close();
}



void BazaWypozyczen::saveToFile() {
	std::ofstream plik(nazwa_pliku, std::ios::trunc);
	for (const auto& wypozyczenie : lista_wypozyczen) {
		plik << wypozyczenie.data_wypozyczenia << " "
			<< wypozyczenie.data_oddania << " "
			<< wypozyczenie.numer_skrytki << " "
			<< wypozyczenie.isbn_ksiazki << " "
			<< wypozyczenie.login_czytelnika << "\n";
	}
	plik.close();
}


void BazaWypozyczen::end_wyp(int isbn) {
	// ZnajdŸ i usuñ wypo¿yczenie z poczekalni
	auto it = std::remove_if(lista_wypozyczen_pocz.begin(), lista_wypozyczen_pocz.end(),
		[isbn](const Wypozyczenie& wyp) { return wyp.isbn_ksiazki == isbn; });

	if (it != lista_wypozyczen_pocz.end()) {
		lista_wypozyczen_pocz.erase(it, lista_wypozyczen_pocz.end());
		saveToFile_pocz(); // Zapisz zmiany do pliku
		std::cout << "Usuniêto wypo¿yczenie dla ISBN: " << isbn << std::endl;
	}
	else {
		std::cerr << "Nie znaleziono wypo¿yczenia dla ISBN: " << isbn << std::endl;
	}
}

void BazaWypozyczen::setWyp() {
	loadFromFile();
}

void BazaWypozyczen::saveWyp() {
	saveToFile();
}

void BazaWypozyczen::saveToFile_pocz() {
	std::ofstream plik(nazwa_pliku_pocz, std::ios::trunc);
	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku do zapisu: " << nazwa_pliku_pocz << std::endl;
		return;
	}

	for (const auto& wypozyczenie : lista_wypozyczen_pocz) {
		plik << wypozyczenie.isbn_ksiazki << " " << wypozyczenie.login_czytelnika << " o\n";
	}
	plik.close();
}

void BazaWypozyczen::przyznanieWyp(string login, int isbn, BazaSkrytek baza_skrytek) {
	if (isBookBorrowed(isbn)) {
		cout << "Ksi¹¿ka o ISBN " << isbn << " jest ju¿ wypo¿yczona.\n";
		return;
	}

	int skr = baza_skrytek.getFirstFree();
	if (skr == -1) {
		cout << "Brak wolnych skrytek\n";
		return;
	}

	// Dodanie wypo¿yczenia do poczekalni

	lista_wypozyczen_pocz.push_back(Wypozyczenie("", "", skr, isbn, login));
	ofstream plik(nazwa_pliku_pocz, ios::app);
	if (plik.is_open()) {
		plik << isbn << " " << login;
		plik.close();
	}
}

std::vector<Wypozyczenie> BazaWypozyczen::getlista_wypozyczen_pocz() {

	loadFromFile_pocz(); // Wczytaj dane z pliku do listy

	/*
	for (auto it : lista_wypozyczen_pocz) {
		cout<<it.isbn_ksiazki<<endl;
	}*/

	return lista_wypozyczen_pocz; // Zwróæ wektor
}

void BazaWypozyczen::add(string dat_wyp, string dat_od, int skr, int num_ks, string log) {
	Wypozyczenie wypozyczenie(dat_wyp, dat_od, skr, num_ks, log);
	lista_wypozyczen.push_back(wypozyczenie);
	saveToFile();
}

bool BazaWypozyczen::isBookBorrowed(int isbn) const {
	// Sprawdzenie w g³ównej liœcie wypo¿yczeñ
	for (const auto& wypozyczenie : lista_wypozyczen) {
		if (wypozyczenie.isbn_ksiazki == isbn) {
			return true;
		}
	}
	// Sprawdzenie w poczekalni
	for (const auto& wypozyczenie : lista_wypozyczen_pocz) {
		if (wypozyczenie.isbn_ksiazki == isbn) {
			return true;
		}
	}
	return false;
}

void BazaWypozyczen::przeniesDoZwrotow(int isbn) {
	auto it = std::find_if(lista_wypozyczen.begin(), lista_wypozyczen.end(),
		[isbn](const Wypozyczenie& wyp) { return wyp.isbn_ksiazki == isbn; });

	if (it != lista_wypozyczen.end()) {
		// Dodaj wypo¿yczenie do listy zwrotów
		lista_wypozyczen_zwroty.push_back(*it);

		// Usuñ wypo¿yczenie z listy aktywnych
		lista_wypozyczen.erase(it);

		// Zapisz zmiany w obu listach
		saveToFile();
		saveToFileZwroty();

		std::cout << "Przeniesiono ISBN: " << isbn << " do listy zwrotów.\n";
	}
	else {
		std::cerr << "Nie znaleziono wypo¿yczenia o ISBN: " << isbn << " w aktywnych wypo¿yczeniach.\n";
	}
}

void BazaWypozyczen::loadFromFileZwroty() {
	lista_wypozyczen_zwroty.clear(); // U¿ywamy tej samej listy, co dla poczekalni zwrotów
	std::ifstream plik(nazwa_pliku_zwrotow);

	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku: " << nazwa_pliku_zwrotow << std::endl;
		return;
	}

	std::string linia;
	while (std::getline(plik, linia)) {
		std::istringstream iss(linia);
		int isbn;
		std::string login;

		if (iss >> isbn >> login) {
			lista_wypozyczen_zwroty.emplace_back("", "", -1, isbn, login);
		}
	}

	plik.close();
}

void BazaWypozyczen::saveToFileZwroty() {
	std::ofstream plik(nazwa_pliku_zwrotow, std::ios::trunc);
	if (!plik.is_open()) {
		std::cerr << "Nie mo¿na otworzyæ pliku do zapisu: " << nazwa_pliku_zwrotow << std::endl;
		return;
	}

	for (const auto& wypozyczenie : lista_wypozyczen_zwroty) {
		plik << wypozyczenie.isbn_ksiazki << " " << wypozyczenie.login_czytelnika << "\n";
	}

	plik.close();
}


void BazaWypozyczen::setZwroty() {
	loadFromFileZwroty();
}

std::vector<Wypozyczenie> BazaWypozyczen::getlista_wypozyczen_zwroty() {
	loadFromFileZwroty(); // Wczytaj dane z pliku do listy
	return lista_wypozyczen_zwroty; // Zwróæ wektor
}

void BazaWypozyczen::usunZwrot(int isbn) {
	auto it = std::find_if(lista_wypozyczen_zwroty.begin(), lista_wypozyczen_zwroty.end(),
		[isbn](const Wypozyczenie& wyp) { return wyp.isbn_ksiazki == isbn; });

	if (it != lista_wypozyczen_zwroty.end()) {
		lista_wypozyczen_zwroty.erase(it);  // Usuñ zwrot z wektora
		saveToFileZwroty();                 // Zapisz zmiany do pliku
	}
	else {
		std::cerr << "Nie znaleziono zwrotu o ISBN: " << isbn << ".\n";
	}
}

void BazaWypozyczen::checkAndBlockLateUsers(BazaUzytkownikow& baza_uzytkownikow) {
	for (auto wyp : lista_wypozyczen) {
		if (isPastDate(wyp.data_oddania)) { // SprawdŸ, czy data oddania minê³a
			std::cerr << "OpóŸnione zwrócenie ksi¹¿ki przez u¿ytkownika: "
				<< wyp.getLogin() << std::endl;

			// Zablokuj u¿ytkownika
			baza_uzytkownikow.blockUser(wyp.getLogin());
		}
	}
}