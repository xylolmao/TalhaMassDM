#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>

using json = nlohmann::json;

class Auth {
private:
	std::string token;
	const char* url = "https://discord.com/api/v9/users/@me";

public:
	Auth(const std::string aToken) {
		token = aToken;
	}

	std::string authenticate() {
		{
			cpr::Response res = cpr::Get(cpr::Url{ url },
				cpr::Header{ {"Authorization", token} });
			if (300 > res.status_code && res.status_code >= 200)
				return token;
		}
		{
			cpr::Response res = cpr::Get(cpr::Url{ url },
				cpr::Header{ {"Authorization", "Bot " + token} });
			if (300 > res.status_code && res.status_code >= 200)
				return "Bot " + token;
		}
		return "";
	}
};

class TalhaMassDM {
private:
	std::string auth;
	json req_json;
	std::vector<std::thread> threads;
	std::string msg;

	void Send(std::string id) {
			cpr::Response r = cpr::Post(cpr::Url{ "https://discord.com/api/v9/channels/" + id + "/messages" },
			cpr::Header {
				   {"Authorization", auth},
				   {"Content-Type", "application/json"}
			   }, cpr::Body{ req_json.dump() });
		switch (r.status_code) {
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
			std::cout << "Successfully DMed " << id << std::endl;
			break;
		case 429:
		{
			json rlJSON = json::parse(r.text);
			std::cout << "Rate Limited For " << rlJSON["retry_after"] << "s" << std::endl;
			break;
		}
		default:
			std::cout << "Couldn't DM " << id << " Status Code: " << r.status_code << std::endl;
			break;
		}
	}

	void SendAll() {
		system("cls || clear");
		system("title Talha Mass DM ~ DMing Friends");

		cpr::Response get_json = cpr::Get(cpr::Url{ "https://discord.com/api/v9/users/@me/channels" },
			cpr::Header{ {"Authorization", auth} });
		json friend_json = json::parse(get_json.text);
		for (int i = 0; i < friend_json.size(); ++i) {
			auto lambda_for_thread = [this, friend_json, i]() { Send(((std::string)friend_json[i]["recipients"][0]["id"]).c_str()); };
			threads.push_back(std::thread(lambda_for_thread));
		}
		
		int friends_dmed_counter;

		for (friends_dmed_counter = 0; friends_dmed_counter < threads.size(); ++friends_dmed_counter) {
			threads[friends_dmed_counter].join();
		}

		std::cout << "DMed " << friends_dmed_counter << " Friends" << std::endl;
	}

public:

	// ugly code but tbs ugly so anyway
	void Run() {
		system("cls & mode 80, 20 & title Talha Mass DM ~ Input");
		std::string token = "";
		while (token == "") {
			std::cout << "Token -> ";
			std::getline(std::cin, token);
			Auth a(token);
			token = a.authenticate();
			system("cls || clear");
		}
		auth = token;
		Sleep(1000);
		std::cout << "Message To Send To All Friends -> ";
		std::getline(std::cin, msg);
		req_json = json::parse("{\"content\": \"" + msg + "\", \"nonce\": \"0\", \"tts\": \"false\"}");
		SendAll();
	}
};

int main() {
	TalhaMassDM* tmDM = new TalhaMassDM;

	tmDM->Run();

	delete tmDM;
}