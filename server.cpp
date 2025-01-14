#include <crow.h>
#include <iostream>
#include <string>
#include "Klasy.h"
#include <unordered_map>
#include <random>
#include <algorithm> 
#include <random>  

using namespace std;

std::string generateSessionID() {
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string sessionID = "";
	for (int i = 0; i < 16; ++i) {
		sessionID += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return sessionID;
}

std::string serve_file(const std::string& file_path) {
	std::ifstream file(file_path, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Nie można otworzyć pliku: " + file_path);
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

int main()
{
	srand(time(nullptr));
	crow::SimpleApp app;

	BazaKsiazek baza_ksiazek;
	BazaUzytkownikow baza_uzytkownikow;
	BazaSkrytek baza_skrytek(10);
	Bibliotekarz bib;
	BazaWypozyczen baza_wypozyczen;
	Czytelnik czytelnik;

	baza_ksiazek.setBooks();
	baza_uzytkownikow.setUsers();
	baza_skrytek.setSkrytki();
	baza_wypozyczen.setWyp();

	//sesje logowania
	std::unordered_map<std::string, std::string> sessions;

	//Sprawdzanie czy użytkownicy oddali książki na czas, co 30 minut
	std::thread([&]() {
		while (true) {
			try {
				std::cout << "[INFO] Sprawdzanie opóźnionych zwrotów..." << std::endl;
				baza_wypozyczen.checkAndBlockLateUsers(baza_uzytkownikow);
			}
			catch (const std::exception& e) {
				std::cerr << "[ERROR] Błąd podczas sprawdzania opóźnionych zwrotów: " << e.what() << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::minutes(30)); // Sprawdzanie co 30 minut
		}
		}).detach();

	CROW_ROUTE(app, "/")([&sessions, &baza_ksiazek](const crow::request& req) {
		std::string sessionID = req.get_header_value("Cookie");
		std::string login = "";

		if (!sessionID.empty() && sessions.count(sessionID)) {
			login = sessions[sessionID];
		}

		auto page = crow::mustache::load("main.html");
		crow::mustache::context ctx;

		ctx["login"] = login;

		// Pobranie książek i wymieszanie ich
		baza_ksiazek.setBooks();
		std::vector<Ksiazka> ksiazki = baza_ksiazek.getBooks();
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(ksiazki.begin(), ksiazki.end(), g);

		// Wybranie maksymalnie 6 książek
		crow::json::wvalue::list ksiazki_list;
		size_t limit = std::min(ksiazki.size(), static_cast<size_t>(6));
		for (size_t i = 0; i < limit; ++i) {
			crow::json::wvalue book_ctx;
			book_ctx["tytul"] = ksiazki[i].tytul;
			book_ctx["autor"] = ksiazki[i].autor;
			book_ctx["okladka"] = ksiazki[i].okladka;
			ksiazki_list.push_back(std::move(book_ctx));
		}

		ctx["ksiazki"] = std::move(ksiazki_list); // Przypisanie listy książek do kontekstu

		return page.render(ctx);
		});

	CROW_ROUTE(app, "/main.html")([&sessions, &baza_ksiazek](const crow::request& req) {
		std::string sessionID = req.get_header_value("Cookie");
		std::string login = "";

		if (!sessionID.empty() && sessions.count(sessionID)) {
			login = sessions[sessionID];
		}

		auto page = crow::mustache::load("main.html");
		crow::mustache::context ctx;

		ctx["login"] = login;

		// Pobranie książek i wymieszanie ich
		baza_ksiazek.setBooks();
		std::vector<Ksiazka> ksiazki = baza_ksiazek.getBooks();
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(ksiazki.begin(), ksiazki.end(), g);

		// Wybranie maksymalnie 6 książek
		crow::json::wvalue::list ksiazki_list;
		size_t limit = std::min(ksiazki.size(), static_cast<size_t>(6));
		for (size_t i = 0; i < limit; ++i) {
			crow::json::wvalue book_ctx;
			book_ctx["tytul"] = ksiazki[i].tytul;
			book_ctx["autor"] = ksiazki[i].autor;
			book_ctx["okladka"] = ksiazki[i].okladka;
			ksiazki_list.push_back(std::move(book_ctx));
		}

		ctx["ksiazki"] = std::move(ksiazki_list); // Przypisanie listy książek do kontekstu

		return page.render(ctx);
		});

	CROW_ROUTE(app, "/logowanie.html")
		([](const crow::request& req) {
		auto page = crow::mustache::load_text("logowanie.html");
		return page;
			});

	CROW_ROUTE(app, "/rejestracja.html")
		([](const crow::request& req) {
		auto page = crow::mustache::load_text("rejestracja.html");
		return page;
			});

	CROW_ROUTE(app, "/wyszukanie.html")
		([](const crow::request& req) {
		auto page = crow::mustache::load_text("wyszukanie.html");
		return page;
			});

	CROW_ROUTE(app, "/main_log.html")([&sessions, &baza_ksiazek](const crow::request& req) {
		std::string sessionID = req.get_header_value("Cookie");
		sessionID.erase(0, 10);//usuwamy prefiks sessionID=
		std::string login;

		if (!sessionID.empty() && sessions.count(sessionID)) {
			login = sessions[sessionID];
		}

		//std::cout << "Login: " << login << std::endl;

		auto page = crow::mustache::load("main_log.html");
		crow::mustache::context ctx;

		ctx["login"] = login;

		// Pobranie książek i wymieszanie ich
		baza_ksiazek.setBooks();
		std::vector<Ksiazka> ksiazki = baza_ksiazek.getBooks();
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(ksiazki.begin(), ksiazki.end(), g);

		// Wybranie maksymalnie 6 książek
		crow::json::wvalue::list ksiazki_list;
		size_t limit = std::min(ksiazki.size(), static_cast<size_t>(6));
		for (size_t i = 0; i < limit; ++i) {
			crow::json::wvalue book_ctx;
			book_ctx["tytul"] = ksiazki[i].tytul;
			book_ctx["autor"] = ksiazki[i].autor;
			book_ctx["okladka"] = ksiazki[i].okladka;
			ksiazki_list.push_back(std::move(book_ctx));
		}

		ctx["ksiazki"] = std::move(ksiazki_list); // Przypisanie listy książek do kontekstu

		return page.render(ctx);
		});

	CROW_ROUTE(app, "/wyszukanie_log.html")
		([&sessions](const crow::request& req) {
		std::string sessionID = req.get_header_value("Cookie");
		sessionID.erase(0, 10);//usuwamy prefiks sessionID=
		std::string login;

		if (!sessionID.empty() && sessions.count(sessionID)) {
			login = sessions[sessionID];
		}

		auto page = crow::mustache::load("wyszukanie_log.html");
		crow::mustache::context ctx;
		ctx["login"] = login;

		return page.render(ctx);
			});

	CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)([&baza_uzytkownikow, &sessions](const crow::request& req) {\
		//cout << req.body << endl;
		auto body = req.body;
	string login = body;
	login.erase(0, 10);
	char c = 34;
	int pos = login.find(c);

	string haslo = body;
	haslo.erase(0, pos + 13);

	login.erase(pos, login.size());
	haslo.erase(0, 11);
	haslo.pop_back();
	haslo.pop_back();

	try {
		if (login == "admin") {
			if (haslo == "admin") {
				std::string sessionID = generateSessionID();
				sessions[sessionID] = login;

				crow::response res(200, "admin");
				res.add_header("Set-Cookie", "sessionID=" + sessionID + "; Path=/; HttpOnly");

				return res;
			}
			else {
				return crow::response(401, "Bledne haslo");
			}
		}
		else if (baza_uzytkownikow.wyszukaj_czy_jest(login)) {
			auto user = baza_uzytkownikow.wyszukaj(login);
			if (user.checkPass(haslo)) {
				std::string sessionID = generateSessionID();
				sessions[sessionID] = login; // Zapisanie sesji w mapie
				//cout << "Zalogowano: " << login << " w sesji: " << sessionID << endl;
				crow::response res(200, "Poprawne logowanie");
				res.add_header("Set-Cookie", "sessionID=" + sessionID + "; Path=/; HttpOnly");

				return res;
			}
			else {
				return crow::response(401, "Bledne haslo");
			}
		}
		else {
			if (baza_uzytkownikow.wyszukaj_czy_jest_zabl(login)) {
				return crow::response(401, "Konto zablokowane. Aby odblokować musisz oddać książkę i uiścić opłatę u bibliotekarza!");
			}
			else {
				return crow::response(404, "Nie znaleziono uzytkownika");
			}
		}
	}
	catch (const std::exception& e) {
		return crow::response(500, e.what());
	}
		});

	CROW_ROUTE(app, "/api/rejestracja").methods(crow::HTTPMethod::POST)([&baza_uzytkownikow](const crow::request& req) {
		auto body = req.body;

		string imie;
		string nazwisko;
		string email;
		string adres;
		string tel;
		string login;
		string haslo;
		int pos = 0;

		imie = body.substr(9, body.length());
		char c = 34;// 34 to kod ASCII znaku "
		pos = imie.find(c);
		nazwisko = imie.substr(pos + 14, imie.length());
		imie = imie.substr(0, pos);
		pos = nazwisko.find(c);
		email = nazwisko.substr(pos + 11, nazwisko.length());
		nazwisko = nazwisko.substr(0, pos);
		pos = email.find(c);
		adres = email.substr(pos + 11, email.length());
		email = email.substr(0, pos);
		pos = adres.find(c);
		tel = adres.substr(pos + 13, adres.length());
		adres = adres.substr(0, pos);
		pos = tel.find(c);
		login = tel.substr(pos + 11, tel.length());
		tel = tel.substr(0, pos);
		pos = login.find(c);
		haslo = login.substr(pos + 11, login.length());
		login = login.substr(0, pos);
		pos = haslo.find(c);
		haslo = haslo.substr(0, pos);

		//std::cout << imie << ", " << nazwisko << ", " << email << ", " << adres << ", " << tel << ", " << login << ", " << haslo << std::endl;
		int telefon = std::stoi(tel);

		try {
			if (baza_uzytkownikow.wyszukaj_czy_jest(login)) {
				return crow::response(409, "User already exists");
			}
			else {
				baza_uzytkownikow.add(imie, nazwisko, email, adres, telefon, login, haslo);
				return crow::response(200, "User registered successfully");
			}
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});

	CROW_ROUTE(app, "/api/wyszukaj").methods(crow::HTTPMethod::POST)([&baza_ksiazek](const crow::request& req) {
		auto body = crow::json::load(req.body);
		if (!body) {
			return crow::response(400, "Invalid JSON format");
		}

		std::string typ = body["typ"].s(); // "tytul" lub "isbn"
		std::string wartosc = body["wartosc"].s();

		try {
			if (typ == "tytul") {
				if (baza_ksiazek.wyszukaj_czy_jest(wartosc)) {
					auto ksiazka = baza_ksiazek.wyszukaj((wartosc));
					crow::json::wvalue result;
					result["tytul"] = ksiazka.tytul;
					result["autor"] = ksiazka.autor;
					result["ISBN"] = ksiazka.ISBN;
					result["okladka"] = ksiazka.okladka;
					return crow::response(result);
				}
				else {
					return crow::response(404, "Nie znaleziono książki o podanym tytule");
				}
			}
			else if (typ == "isbn") {
				int isbn = std::stoi(wartosc);
				if (baza_ksiazek.wyszukaj_czy_jest(isbn)) {
					auto ksiazka = baza_ksiazek.wyszukaj(isbn);
					crow::json::wvalue result;

					result["tytul"] = ksiazka.tytul;
					result["autor"] = ksiazka.autor;
					result["ISBN"] = ksiazka.ISBN;
					result["okladka"] = ksiazka.okladka;
					return crow::response(result);
				}
				else {
					return crow::response(404, "Nie znaleziono książki o podanym ISBN");
				}
			}
			else {
				return crow::response(400, "Nieprawidłowy typ wyszukiwania");
			}
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});

	CROW_ROUTE(app, "/api/wyszukaj_log").methods(crow::HTTPMethod::POST)([&baza_ksiazek](const crow::request& req) {
		auto body = crow::json::load(req.body);
		if (!body) {
			return crow::response(400, "Invalid JSON format");
		}

		std::string typ = body["typ"].s(); // "tytul" lub "isbn"
		std::string wartosc = body["wartosc"].s();

		try {
			if (typ == "tytul") {
				if (baza_ksiazek.wyszukaj_czy_jest(wartosc)) {
					auto ksiazka = baza_ksiazek.wyszukaj((wartosc));
					crow::json::wvalue result;
					result["tytul"] = ksiazka.tytul;
					result["autor"] = ksiazka.autor;
					result["ISBN"] = ksiazka.ISBN;
					result["okladka"] = ksiazka.okladka;
					return crow::response(result);
				}
				else {
					return crow::response(404, "Nie znaleziono książki o podanym tytule");
				}
			}
			else if (typ == "isbn") {
				int isbn = std::stoi(wartosc);
				if (baza_ksiazek.wyszukaj_czy_jest(isbn)) {
					auto ksiazka = baza_ksiazek.wyszukaj(isbn);
					crow::json::wvalue result;
					result["tytul"] = ksiazka.tytul;
					result["autor"] = ksiazka.autor;
					result["ISBN"] = ksiazka.ISBN;
					result["okladka"] = ksiazka.okladka;
					return crow::response(result);
				}
				else {
					return crow::response(404, "Nie znaleziono książki o podanym ISBN");
				}
			}
			else {
				return crow::response(400, "Nieprawidłowy typ wyszukiwania");
			}
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});

	CROW_ROUTE(app, "/static/<string>")
		([](const std::string& filename) {
		std::ifstream file("static/" + filename, std::ios::binary);
		if (!file.is_open()) {
			return crow::response(404);
		}
		std::ostringstream content;
		content << file.rdbuf();
		return crow::response(content.str());
			});

	CROW_ROUTE(app, "/bib_main.html")([&baza_uzytkownikow](const crow::request&) {
		auto page = crow::mustache::load("bib_main.html");
		crow::mustache::context ctx;

		// Pobierz listę użytkowników niezablokowanych
		baza_uzytkownikow.setUsers();
		auto lista_czytelnikow = baza_uzytkownikow.getlista_czytelnikow();

		crow::json::wvalue::list niezablokowani_list;

		for (auto czytelnik : lista_czytelnikow) {
			crow::json::wvalue item;
			item["login"] = czytelnik.getLogin();
			item["imie"] = czytelnik.getImie();
			item["nazwisko"] = czytelnik.getNazwisko();
			niezablokowani_list.push_back(std::move(item));
		}

		// Pobierz listę użytkowników zablokowanych
		baza_uzytkownikow.loadBlockedFromFile();
		auto lista_zablokowanych = baza_uzytkownikow.getlista_zablokowanych();
		crow::json::wvalue::list zablokowani_list;

		for (auto czytelnik : lista_zablokowanych) {
			crow::json::wvalue item;
			item["login"] = czytelnik.getLogin();
			item["imie"] = czytelnik.getImie();
			item["nazwisko"] = czytelnik.getNazwisko();
			zablokowani_list.push_back(std::move(item));
		}

		ctx["niezablokowani"] = std::move(niezablokowani_list);
		ctx["zablokowani"] = std::move(zablokowani_list);

		return page.render(ctx);
		});



	CROW_ROUTE(app, "/bib_book.html")
		([&baza_ksiazek](const crow::request& req) {


		//Pobieranie i wyświetlanie listy książek
		std::vector<Ksiazka> ksiazki = baza_ksiazek.getBooks();
		crow::json::wvalue::list ksiazki_list;

		auto page = crow::mustache::load("bib_book.html");
		crow::mustache::context ctx;

		for (const auto& ksiazka : ksiazki) {
			crow::json::wvalue book_ctx;
			book_ctx["tytul"] = ksiazka.tytul;
			book_ctx["autor"] = ksiazka.autor;
			book_ctx["isbn"] = ksiazka.ISBN;
			book_ctx["okladka"] = ksiazka.okladka;
			ksiazki_list.push_back(std::move(book_ctx));
		}

		ctx["ksiazki"] = std::move(ksiazki_list); // Przypisanie listy książek do kontekstu

		return page.render(ctx);
			});

	CROW_ROUTE(app, "/api/bib_book").methods(crow::HTTPMethod::POST)([&bib, &baza_ksiazek](const crow::request& req) {
		try {

			//Ze strony otrzymujemy JSON wyglądający tak: {"tytul":"Fenomenologia","autor":"Hegel","isbn":"123","okladka":"okladka.jpg"}
			//musimy go poprzycinać

			std::string body = req.body;

			//jako że początek body to {"tytul":" to musimy wyciąć pierwsze 10 znaków

			string tytul = body.substr(10, body.length());

			//wyszukujemy " w stringu, bo to znacznik końca tytułu
			int i = tytul.find_first_of('"');

			//kopiujemy zawartość tytułu po " i 11 innych znakach, gdzie zacznie się autor
			string autor = tytul.substr(i + 11, tytul.length());

			//docinamy żeby był sam tytuł
			tytul = tytul.substr(0, i);
			//cout << "Tytul: " << tytul << endl;

			//powtarzamy to dla wszystkich danych
			i = autor.find_first_of('"');
			string ISBN = autor.substr(i + 10, autor.length());
			autor = autor.substr(0, i);
			//cout << "Autor: " << autor << endl;

			i = ISBN.find_first_of('"');
			string okladka = ISBN.substr(i + 13, ISBN.length());
			ISBN = ISBN.substr(0, i);
			//cout << "ISBN: " << ISBN << endl;
			okladka = okladka.substr(0, okladka.length() - 2);
			//cout << "Okladka: " << okladka << endl;


			//tworzymy obiekt książki
			if (baza_ksiazek.wyszukaj_czy_jest(std::stoi(ISBN))) {
				return crow::response(400, R"({"error": "Książka o podanym ISBN już istnieje"})");
			}
			else {
				bib.dodanie_ksiazki(tytul, autor, std::stoi(ISBN), okladka, baza_ksiazek);

				return crow::response(200);
			}
		}
		catch (const std::exception& e) {
			// Obsługa błędów
			std::cerr << "Błąd: " << e.what() << std::endl;
			return crow::response(400, R"({"error": "Wystąpił błąd podczas przetwarzania danych"})");
		}
		});


	CROW_ROUTE(app, "/api/bib_book/<int>")
		.methods("DELETE"_method)([&baza_ksiazek](int isbn) {
		if (baza_ksiazek.wyszukaj_czy_jest(isbn)) {
			baza_ksiazek.usun_ksiazke(isbn);
			cout << "Książka: "<< isbn <<" została usunięta" << endl;
			return crow::response(200, "Książka została usunięta");
		}
		return crow::response(404, "Nie znaleziono książki o podanym ISBN");
			});



	CROW_ROUTE(app, "/bib_wyp.html")
		([](const crow::request& req) {
		auto page = crow::mustache::load_text("bib_wyp.html");
		return page;
			});


	CROW_ROUTE(app, "/api/borrow").methods("POST"_method)([&sessions, &baza_ksiazek, &baza_wypozyczen, &baza_skrytek, &czytelnik](const crow::request& req) {
		try {
			auto body = crow::json::load(req.body);
			if (!body) {
				return crow::response(400, "Invalid JSON");
			}

			std::string isbn = body["isbn"].s();
			int isbn_int = std::stoi(isbn);

			// Pobranie loginu użytkownika z ciasteczka
			std::string sessionID = req.get_header_value("Cookie");
			sessionID.erase(0, 10); // Usunięcie prefiksu "sessionID="
			std::string login = sessions.count(sessionID) ? sessions[sessionID] : "";

			if (login.empty()) {
				return crow::response(401, "Nie jesteś zalogowany.");
			}

			// Sprawdzenie, czy książka jest już wypożyczona
			if (baza_wypozyczen.isBookBorrowed(isbn_int)) {
				return crow::response(400, "Książka jest już wypożyczona.");
			}

			czytelnik.wypozycz(isbn_int, login, baza_ksiazek, baza_wypozyczen, baza_skrytek);
			return crow::response(200, "Książka została dodana do poczekalni.");
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});



	CROW_ROUTE(app, "/api/get_queue")
		([&baza_wypozyczen, &baza_ksiazek]() {
		try {
			std::vector<Wypozyczenie> poczekalnia = baza_wypozyczen.getlista_wypozyczen_pocz();

			crow::json::wvalue result;
			crow::json::wvalue::list queue_list;

			for (auto wyp : poczekalnia) {
				crow::json::wvalue item;
				try {
					Ksiazka ksiazka = baza_ksiazek.wyszukaj(wyp.getISBN());
					item["isbn"] = wyp.getISBN();
					item["tytul"] = ksiazka.tytul;
					item["autor"] = ksiazka.autor;
					item["login"] = wyp.getLogin();
				}
				catch (const std::exception&) {
					item["isbn"] = wyp.getISBN();
					item["tytul"] = "Nieznany";
					item["autor"] = "Nieznany";
					item["login"] = wyp.getLogin();
				}
				queue_list.push_back(std::move(item));
			}

			result["queue"] = std::move(queue_list);
			return crow::response(200, result);
		}
		catch (const std::exception& e) {
			crow::json::wvalue error_response;
			error_response["error"] = e.what();
			return crow::response(500, error_response);
		}
			});

	CROW_ROUTE(app, "/api/manage_queue/<int>/<string>")
		.methods("POST"_method)([&baza_wypozyczen, &bib,&baza_skrytek,&baza_uzytkownikow, &baza_ksiazek](int isbn, std::string action) {
		if (action == "accept") {
			try {
				bib.akceptacja_wyp(baza_wypozyczen, isbn, baza_skrytek, baza_uzytkownikow, baza_ksiazek);
				return crow::response(200, "Wypożyczenie zaakceptowane.");
			}
			catch (const std::exception& e) {
				return crow::response(500, e.what());
			}
		}
		else if (action == "delete") {
			try {
				baza_wypozyczen.end_wyp(isbn);
				return crow::response(200, "Wypożyczenie usunięte.");
			}
			catch (const std::exception& e) {
				return crow::response(500, e.what());
			}
		}
		return crow::response(400, "Niepoprawna akcja.");
			});

	CROW_ROUTE(app, "/user.html")([&sessions, &baza_uzytkownikow, &baza_ksiazek, &baza_wypozyczen](const crow::request& req) {
		std::string sessionID = req.get_header_value("Cookie");
		sessionID.erase(0, 10); // Usunięcie prefiksu "sessionID="
		std::string login;

		if (!sessionID.empty() && sessions.count(sessionID)) {
			login = sessions[sessionID];
		}
		else {
			return crow::response(401, "Nie jesteś zalogowany."); // crow::response
		}

		if (!baza_uzytkownikow.wyszukaj_czy_jest(login)) {
			return crow::response(404, "Nie znaleziono użytkownika."); // crow::response
		}

		auto user = baza_uzytkownikow.wyszukaj(login);
		auto page = crow::mustache::load("user.html");
		crow::mustache::context ctx;

		ctx["imie"] = user.getImie();
		ctx["nazwisko"] = user.getNazwisko();
		ctx["email"] = user.getEmail();

		crow::json::wvalue::list wypozyczenia_list;
		for (auto wyp : baza_wypozyczen.lista_wypozyczen) {
			if (wyp.getLogin() == login) {
				crow::json::wvalue wyp_ctx;
				wyp_ctx["tytul"] = baza_ksiazek.wyszukaj(wyp.getISBN()).tytul;
				wyp_ctx["autor"] = baza_ksiazek.wyszukaj(wyp.getISBN()).autor;
				wyp_ctx["isbn"] = wyp.getISBN();
				wyp_ctx["data_oddania"] = wyp.data_oddania; // Dodanie daty oddania
				wypozyczenia_list.push_back(std::move(wyp_ctx));
			}
		}

		ctx["wypozyczenia"] = std::move(wypozyczenia_list);
		return crow::response(page.render(ctx)); // crow::response
		});



	CROW_ROUTE(app, "/api/confirm_return/<int>").methods("POST"_method)(
		[&baza_wypozyczen, &baza_skrytek](int isbn) {
			try {
				// Znajdź skrytkę na podstawie ISBN
				int locker_id = -1;
				for (auto skrytka : baza_skrytek.lista_skrytek) {
					if (skrytka.getNumerWypozyczenia() == isbn) {
						locker_id = skrytka.getID();
						break;
					}
				}

				if (locker_id == -1) {
					return crow::response(400, "Nie znaleziono skrytki dla ISBN.");
				}

				baza_skrytek.zwolnij(locker_id);
				baza_wypozyczen.end_wyp(isbn);

				return crow::response(200, "Zwrot potwierdzony i skrytka zwolniona.");
			}
			catch (const std::exception& e) {
				return crow::response(500, e.what());
			}
		});



	CROW_ROUTE(app, "/api/get_return_queue")
		([&baza_wypozyczen, &baza_ksiazek]() {
		try {
			baza_wypozyczen.loadFromFileZwroty();
			std::vector<Wypozyczenie> zwroty = baza_wypozyczen.getlista_wypozyczen_zwroty();

			crow::json::wvalue result;
			crow::json::wvalue::list queue_list;

			for (auto wyp : zwroty) {
				crow::json::wvalue item;
				try {
					Ksiazka ksiazka = baza_ksiazek.wyszukaj(wyp.getISBN());
					item["isbn"] = wyp.getISBN();
					item["tytul"] = ksiazka.tytul;
					item["autor"] = ksiazka.autor;
					item["login"] = wyp.getLogin();
				}
				catch (const std::exception&) {
					item["isbn"] = wyp.getISBN();
					item["tytul"] = "Nieznany";
					item["autor"] = "Nieznany";
					item["login"] = wyp.getLogin();
				}
				queue_list.push_back(std::move(item));
			}

			result["queue"] = std::move(queue_list);
			return crow::response(200, result);
		}
		catch (const std::exception& e) {
			crow::json::wvalue error_response;
			error_response["error"] = e.what();
			return crow::response(500, error_response);
		}
			});




	CROW_ROUTE(app, "/api/return/<int>").methods("POST"_method)(
		[&baza_wypozyczen, &baza_skrytek](int isbn) {
			try {
				int locker_id = baza_skrytek.getFirstFree();
				if (locker_id == -1) {
					return crow::response(400, "Brak wolnych skrytek.");
				}

				// Przeniesienie wypożyczenia do zwrotów
				baza_wypozyczen.przeniesDoZwrotow(isbn);

				// Zwrócenie informacji o skrytce
				crow::json::wvalue response;
				response["skrytka"] = locker_id;
				return crow::response(200, response);
			}
			catch (const std::exception& e) {
				return crow::response(500, e.what());
			}
		});


	CROW_ROUTE(app, "/api/accept_return/<int>").methods("POST"_method)(
		[&baza_wypozyczen, &baza_skrytek](int isbn) {
			try {
				// Usuń zwrot z bazy zwrotów
				baza_wypozyczen.usunZwrot(isbn);

				// Zwolnij skrytkę, jeśli była zajęta
				for (auto skrytka : baza_skrytek.lista_skrytek) {
					if (skrytka.getNumerWypozyczenia() == isbn) {
						baza_skrytek.zwolnij(skrytka.getID());
						break;
					}
				}

				return crow::response(200, "Zwrot zaakceptowany i usunięty.");
			}
			catch (const std::exception& e) {
				return crow::response(500, e.what());
			}
		});

	CROW_ROUTE(app, "/bib_wyp_akt.html")
		([](const crow::request& req) {
		auto page = crow::mustache::load_text("bib_wyp_akt.html");
		return page;
			});

	CROW_ROUTE(app, "/api/get_current_borrows")([&baza_wypozyczen, &baza_ksiazek]() {
		try {
			crow::json::wvalue result;
			crow::json::wvalue::list borrow_list;

			// Pobierz aktualne wypożyczenia z bazy
			std::vector<Wypozyczenie> lista_wypozyczen = baza_wypozyczen.lista_wypozyczen;

			for (auto wyp : lista_wypozyczen) {
				crow::json::wvalue borrow;

				try {
					// Wyszukaj szczegóły książki
					Ksiazka ksiazka = baza_ksiazek.wyszukaj(wyp.isbn_ksiazki);

					borrow["tytul"] = ksiazka.tytul;
					borrow["autor"] = ksiazka.autor;
					borrow["isbn"] = ksiazka.ISBN;
					borrow["login"] = wyp.getLogin();
					borrow["data_wypozyczenia"] = wyp.data_wypozyczenia;
					borrow["data_oddania"] = wyp.data_oddania;

				}
				catch (const std::exception& e) {
					// Obsługa przypadku, gdy książka nie została znaleziona
					borrow["tytul"] = "Nieznany";
					borrow["autor"] = "Nieznany";
					borrow["isbn"] = wyp.isbn_ksiazki;
					borrow["login"] = wyp.getLogin();
					borrow["data_wypozyczenia"] = wyp.data_wypozyczenia;
					borrow["data_oddania"] = wyp.data_oddania;
				}

				borrow_list.push_back(std::move(borrow));
			}

			result["borrows"] = std::move(borrow_list);
			return crow::response(200, result);

		}
		catch (const std::exception& e) {
			crow::json::wvalue error_response;
			error_response["error"] = e.what();
			return crow::response(500, error_response);
		}
		});



	CROW_ROUTE(app, "/api/users")
		.methods("GET"_method)([&baza_uzytkownikow]() {
		crow::json::wvalue result;
		crow::json::wvalue::list users_list;

		for (auto user : baza_uzytkownikow.getlista_czytelnikow()) {
			crow::json::wvalue user_data;
			user_data["id"] = user.getID();
			user_data["login"] = user.getLogin();
			user_data["imie"] = user.getImie();
			user_data["nazwisko"] = user.getNazwisko();
			user_data["email"] = user.getEmail();
			users_list.push_back(std::move(user_data));
		}

		result["users"] = std::move(users_list);
		return crow::response(200, result);
			});

	//usuwanie użytkonika
	CROW_ROUTE(app, "/api/users/<string>")
		.methods("DELETE"_method)([&baza_uzytkownikow](std::string login) {
		baza_uzytkownikow.usun(login);
		baza_uzytkownikow.saveToFile();
		return crow::response(200, "Użytkownik został usunięty");
			});

	//blokowanie
	CROW_ROUTE(app, "/api/block_user/<string>").methods("POST"_method)([&baza_uzytkownikow](const crow::request&, const string& login) {
		try {
			cout<<login<<endl;
			baza_uzytkownikow.blockUser(login);
			return crow::response(200, "Użytkownik zablokowany.");
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});

	//odblokowanie
	CROW_ROUTE(app, "/api/unblock_user/<string>").methods("POST"_method)([&baza_uzytkownikow](const crow::request&, const string& login) {
		try {
			baza_uzytkownikow.unblockUser(login);
			return crow::response(200, "Użytkownik odblokowany.");
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});


	//prolongata
	CROW_ROUTE(app, "/api/prolong/<int>").methods("POST"_method)([&baza_wypozyczen](int isbn) {
		try {
			// Znajdź wypożyczenie
			auto& wypozyczenie = baza_wypozyczen.lista_wypozyczen;
			auto it = std::find_if(wypozyczenie.begin(), wypozyczenie.end(),
				[isbn](Wypozyczenie wyp) { return wyp.getISBN() == isbn; });

			if (it == wypozyczenie.end()) {
				return crow::response(404, "Nie znaleziono wypożyczenia");
			}

			if (it->data_oddania.back() == '*') { // Sprawdzamy, czy prolongata już była
				return crow::response(400, "Prolongata już była użyta");
			}

			// Dodaj 30 dni do daty oddania
			it->prolong(30, it->data_oddania);

			baza_wypozyczen.saveWyp();

			return crow::response(200, "Prolongata zaakceptowana");
		}
		catch (const std::exception& e) {
			return crow::response(500, e.what());
		}
		});

	app.port(8080).multithreaded().run();
}
