#include <windows.h>
#include <clocale>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>
#include <cpr/cpr.h>
#include <string>
#include <nlohmann/json.hpp>
#include <string>
#include <locale>
#include <clocale>
#include <vector>
#include <codecvt>

using json = nlohmann::json;



const std::string API_KEY = "0dae9691-c545-42b7-ad0f-c5f21de8f51f";
const std::string STATION_CACHE =  "cache/stations.json";
const std::string WAYS_CACHE =  "cache/ways.json";



void load_station_cache(std::map<std::string, std::string>& station_cache,std::map<std::string, json>& ways_cache) {
    std::ifstream f(STATION_CACHE);
    std::ifstream w(WAYS_CACHE);
    if (f) station_cache = json::parse(f).get<std::map<std::string, std::string>>();
    if(w) ways_cache = json::parse(w).get<std::map<std::string, json>>();
}

void save_station_cache(std::map<std::string, std::string>& station_cache,std::map<std::string, json>& ways_cache) {
    std::ofstream f(STATION_CACHE);
    f.clear();
    f << json(station_cache).dump(4);
    std::ofstream w(WAYS_CACHE);
    w.clear();
    w << json(ways_cache).dump(4);
}

std::string get_station_code(const std::string& city,std::map<std::string, std::string>& station_cache) {
   if (station_cache.count(city)) return station_cache[city];
    auto response = cpr::Get(
        cpr::Url{"https://api.rasp.yandex.net/v3.0/stations_list/"},
        cpr::Parameters{
            {"apikey", API_KEY},
            {"format", "json"},
            {"lang", "ru_RU"}
        }

    );
    std::string code;

    if (response.status_code != 200) {
        std::cerr << "API error" << response.text << std::endl;
    }

    json j = json::parse(response.text);

    for (auto country : j["countries"]) {
        for (auto region : country["regions"]) {
            for (auto settlement : region["settlements"]) {
                if (settlement.contains("title") && settlement["title"] == city) {
                    code = settlement["codes"]["yandex_code"];
                    break;
                }
            }
            if (!code.empty())
                break;
        }
        if (!code.empty())
            break;
    }

    station_cache[city] = code;

    return code;
}


json get_routes(const std::string& from_code, const std::string& to_code, const std::string& date,std::map<std::string, json>& ways_cache) {
    std::string way = from_code + "|" + to_code;
    if(ways_cache.contains(way)){
        return ways_cache[way];
    }

    auto response = cpr::Get(
        cpr::Url{"https://api.rasp.yandex.net/v3.0/search/"}, 
        cpr::Parameters{
            {"apikey", API_KEY},
            {"from", from_code},    
            {"to", to_code},        
            {"date", date},     
            {"transfers", (0)?"true":"false"} 
        }
    );

    if (response.status_code != 200) {
        std::cerr << "API Response: " << response.text << std::endl;
    }

    json result = json::parse(response.text);
    ways_cache[way] = result;

    return result;
}
int count_in_json(const json& j,std::string val){
    int count = 0;
    for (const auto& segment : j["segments"]) {

        if (segment.contains("thread") && segment["thread"].contains(val)) {
            ++count;
        }
    }
    return count;
}


void print_routes(const json& routes,int i) {
    if (!routes.contains("segments")) {
        std::cerr << "Некорректный формат" << std::endl;
        return;
    }
    int j = 0;
    for (const auto & segment : routes["segments"]) {
        if(j!=i){j++;continue;}
        if (segment.contains("details") && segment["details"].size() - 2 > 100) {
            continue;
        }
        if (segment.contains("details")) {

            std::cout << "Маршрут с "<< segment["details"].size() - 2 << " пересадками:\n";
            std::cout << "От: " << segment["departure_from"]["title"] << "\n";
            std::cout << "До: " << segment["arrival_to"]["title"] << "\n";
            std::cout << "Длительность: " << segment.value("duration", "N/A") << " ч\n";
            std::cout << "Типы транспорта: ";
            for (const auto& type : segment["transport_types"]) {
                std::cout << type<< " ";
            }
            std::cout << "  Пересадка в " << segment["details"][1]["transfer_point"]["title"] << ":\n";
            std::cout << "    Продолжительность: " << segment["details"][1]["duration"] << "\n";
            std::cout << "    От: " << segment["details"][1]["transfer_from"]["title"] << "\n";
            std::cout << "    К: " << segment["details"][1]["transfer_to"]["title"]<< "";
            std::cout << "\n-------------------\n";
        }
        else {
            std::cout << "Прямой маршрут:\n";
            std::cout << "Тип: " << segment["thread"]["transport_type"] << "\n";
            std::cout << "Отправление: " << segment["departure"] << "\n";
            std::cout << "Прибытие: " << segment["arrival"] << "\n";
            std::cout << "Длительность: " << segment["duration"] << " мин\n";
            std::cout << "Отправление из: " << segment["from"]["title"] << "\n";
            std::cout << "Прибытие в: " << segment["to"]["title"] << "\n";
            std::cout << "Номер рейса: " << segment["thread"]["number"] << "\n";
            std::cout << "--------------------------------\n";
        }
        j++;
    
    }
}

std::pair<int,int> get_time(json& routes){
    std::pair<int,int>ans;
    int duration = 1e7;
    int i = 0;
    for (const auto & segment : routes["segments"]) {
        
        if (segment.contains("details")) {
             if(duration >  segment.value("duration", 1e7)){
                duration = segment.value("duration", 1e7);
                ans.second = i;
             }
           
            }
        else {
    
            if(duration >  segment.value("duration", 1e7)){
                duration = segment.value("duration", 1e7);
                ans.second = i;
             }
            
        }
        i++;
    }
    ans.first = duration;
    return ans;
}

int get_distance(const std::string& from, const std::string& to,std::string date,int i,int j,std::map<std::pair<int,int>,json>& ways_final,std::map<std::pair<int,int>,int>& best_way,std::map<std::string, std::string>& station_cache,std::map<std::string, json>& ways_cache) {
    std::string from_code = get_station_code(from,station_cache);
    std::string to_code = get_station_code(to,station_cache);
    json route = get_routes(from_code, to_code, date,ways_cache);
    std::pair<int,int> time = get_time(route);
    ways_final[{i,j}] = route;
    best_way[{i,j}] = time.second;
 
    return time.first; 
}
struct DPState {
    int mask;
    int current;
    int distance;
    std::vector<int> path;
    
    DPState(int m = 0, int c = 0, int d = INT_MAX, std::vector<int> p = {}) 
        : mask(m), current(c), distance(d), path(p) {}
};


int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    std::map<std::string, std::string> station_cache;
    std::map<std::pair<int,int>,json> ways_final;
    std::map<std::pair<int,int>,int> best_way;
    std::map<std::string, json> ways_cache;
    std::string date;


    std::vector<std::string> utf8Args;
    for (int i = 0; i < argc; ++i) {
        utf8Args.push_back(argv[i]);
    }

    date = utf8Args[1];
    load_station_cache(station_cache,ways_cache);


    
    
    std::ifstream input("cities.txt");
    int N = std::stoi(utf8Args[2]);

    std::vector<std::string> cities(N);
    for (auto& city : cities) input >> city;
    
    std::vector<std::vector<int>> dist(N, std::vector<int>(N));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            dist[i][j] = get_distance(cities[i], cities[j],date,i,j,ways_final,best_way,station_cache,ways_cache);
        }
    }


    std::vector<std::vector<DPState>> dp(1 << N, std::vector<DPState>(N, {0, 0, INT_MAX, {}}));
    
    dp[1 << 0][0] = {1 << 0, 0, 0, {0}};
    
    for (int mask = 1; mask < (1 << N); ++mask) {
        for (int u = 0; u < N; ++u) {
            if (!(mask & (1 << u)) || dp[mask][u].distance == INT_MAX) continue;
            
            for (int v = 0; v < N; ++v) {
                if (mask & (1 << v)) continue;
                
                int new_mask = mask | (1 << v);
                int new_dist = dp[mask][u].distance + dist[u][v];
                
                if (new_dist < dp[new_mask][v].distance) {
                    dp[new_mask][v] = {new_mask, v, new_dist, dp[mask][u].path};
                    dp[new_mask][v].path.push_back(v);
                }
            }
        }
    }
    
    
    int final_mask = (1 << N) - 1;
    int min_dist = INT_MAX;
    std::vector<int> best_path;
    
    for (int i = 0; i < N; ++i) {
        if (i == N-1 && dp[final_mask][i].distance < min_dist) {
            min_dist = dp[final_mask][i].distance;
            best_path = dp[final_mask][i].path;
        }
    }
    
    
    std::cout << "Кратчайший маршрут: " << std::endl;

    for (size_t i = 0; i < best_path.size(); ++i) {
        std::cout << cities[best_path[i]];
        if (i < best_path.size()-1) std::cout << " -> ";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < best_path.size()-1; ++i) {
        print_routes(ways_final[{best_path[i],best_path[i+1]}],best_way[{best_path[i],best_path[i+1]}]);
    }
    
    std::cout <<std::endl<< "Время: " << min_dist/3600;


    save_station_cache(station_cache,ways_cache);



    return 0;
}