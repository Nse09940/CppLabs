#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
 
int st = 0;
long long fin = 1e12;
 
struct log {
    int status = 0;
    std::string request = "";
    int time = 807220800; // 1 july 1995
    std::string anlog = "";
 
    int hash() {
        long long p = 1;
        long long an = 0;
        for (int i = 0; i < request.size(); i++) {
            an = (an * 30 + request[i]) % 999983;
        }
        return an;
    }
};
 
struct TArgs {
    const char *input_path = "access_log.txt";
    char *output_path = "out.txt";
    char *arr[20];
};
 
log parse_log(std::string x) {
    log ans;
    for (int i = 0; i < x.size(); ++i) {
 
        if (x[i] == '[') {
            if (x[i + 27] != ']') {
                ans.status = -9;
                return ans;
            }
            if (x[i + 1] == '0') {
                ans.time += ((x[i + 2] - '0') - 1) * 24 * 60 * 60;
            } else {
                if (x[i + 1] - '0' < 0 || x[i + 1] - '0' > 9 || x[i + 2] - '0' < 0 || x[i + 2] - '0' > 9) {
                    ans.status = -9;
                    return ans;
                }
                ans.time += ((10 * (x[i + 1] - '0')) + (x[i + 2] - '0') - 1) * 24 * 60 * 60;
            }
 
            if (x[i + 13] == '0') {
                ans.time += (x[i + 14] - '0') * 60 * 60;
            } else {
                if (x[i + 13] - '0' < 0 || x[i + 14] - '0' > 9 || x[i + 14] - '0' < 0 || x[i + 13] - '0' > 9) {
                    ans.status = -9;
                    return ans;
                }
                ans.time += ((10 * (x[i + 13] - '0')) + (x[i + 14] - '0') - 1) * 60 * 60;
            }
 
 
            if (x[i + 16] == '0') {
                ans.time += (x[i + 17] - '0') * 60;
            } else {
 
                if (x[i + 16] - '0' < 0 || x[i + 16] - '0' > 9 || x[i + 17] - '0' < 0 || x[i + 17] - '0' > 9) {
                    ans.status = -9;
                    return ans;
                }
                ans.time += ((10 * (x[i + 16] - '0')) + (x[i + 17] - '0') - 1) * 60;
            }
 
 
            if (x[i + 19] == '0') {
                ans.time += x[i + 20] - '0';
            } else {
 
                if (x[i + 19] - '0' < 0 || x[i + 19] - '0' > 5 || x[i + 20] - '0' < 0 || x[i + 20] - '0' > 9) {
                    ans.status = -9;
                    return ans;
                }
                ans.time += ((10 * (x[i + 19] - '0')) + (x[i + 20] - '0') - 1);
            }
 
 
            break;
        }
    }
 
    int cnt = 0;
    std::string st;
    std::string rq;
    std::string namelog;
    for (int i = x.size() - 1; i >= 0; --i) {
        if (x[i] == ' ')
            cnt += 1;
        else if (cnt == 1) {
            st = x[i] + st;
        }
        if (cnt == 2 || cnt == 3) {
            rq = x[i] + rq;
        }
    }
    if (st.size() == 0 || rq.size() == 0) {
        ans.status = -9;
        return ans;
    }
    int k = 0;
    while (x[k] != ' ') {
        namelog = namelog + x[k];
        k++;
    }
 
    //std::cout << 88 << std::endl;
    ans.status = stoi(st);
    ans.request = rq;
    ans.anlog = namelog;
    return ans;
}
 
void stats(TArgs *args, int n) { //req_f
    std::ifstream in(args->input_path);
    std::ofstream out(args->output_path);
    std::string line;
    std::vector<std::pair<int, std::string>> data(1000000);
    int ll = 0;
    while (std::getline(in, line)) {
 
        log cur = parse_log(line);
        if (cur.time < st || cur.time > fin) {
            continue;
        }
        if (cur.status / 100 == 5) {
            data[cur.hash()].first++;
            data[cur.hash()].second = cur.request; //вся строка = line
        }
        
    }

    if (data.size() == 0) {
        printf("error stats");
        return;
    }
    std::sort(data.begin(), data.end());
    std::reverse(data.begin(), data.end());
    for (int i = 0; i < std::min(n, (int) data.size()); ++i) {
        out << data[i].second << std::endl;
    }
    in.close();
    out.close();
}
 
void okno(TArgs *args, int t) {
    //std::cout << t << std::endl;
    int ans = 0;
    int l = 0, r = 0;
    std::vector<int> window;
    std::fstream in(args->input_path);
    if (!in.is_open()) {
        std::cout << "error";
        return;
    }
    window.push_back(1);
   // int o = 100;
    while (true) {
       // o++;
        if (window.back() - window.front() > t) {
            window.erase(window.begin());
        } else {
            std::string x;
            if (!std::getline(in, x)) {
                break;
            }
            if (ans < window.size()) {
         		 ans = window.size();
                 //std::cout << window.back() << ' ' <<  window.front() << ' ' << window.back() - window.front() << ' ' << x << std::endl;
           		 l = window.front();
           		 r = window.back();
        	}
            log a = parse_log(x);
            if (a.time < st || a.time > fin)continue;
            //std::cout <<window.front() << ' ' << window.back()<<std::endl;
            window.push_back(a.time);
 			
        }
        //if(o == 20000)break;
 
    }
    std::cout << l << ' ' << r;
}
 
void time_st(int t) {
    st = t;
}
 
void time_fin(int t) {
    fin = t;
}
 
void print(TArgs *args) {
 
    std::string line;
    std::ifstream in(
            args->output_path); // открываем файл вывода, чтобы потом дублировать вывод запросов из него(то, что было в output)
    if (!in.is_open()) {
        printf("error file is crashed");
        return;
    }
    int c = 0;
    while (std::getline(in, line)) {
        c++;
        if (c > st && c < fin)
            std::cout << line << std::endl;
    }
    in.close();
}
 
void file_path(TArgs *args, std::string path) { 
    args->output_path = &path[0];
}
 
// ?|^ передает путь к файлу
bool ReadArgs(TArgs *args, int argc, char *argv[]) {
    std::string nazvanie = "";
    std::string chislo = "";
    for (int i = 1; i < argc - 1; ++i) {
        
        if (strcmp(args->arr[1], "AnalyzeLog") != 0) {
            printf("error input readArgs");
            return false;
        }
        if (i == 1)
            continue;
        if (strcmp(argv[i], "-o") == 0) {
            file_path(args, args->arr[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-p") == 0 or strcmp(argv[i], "--print") == 0) {
            print(args);
        } else if (strcmp(argv[i], "-s") == 0) {
            stats(args, atoi(args->arr[i + 1]));
            i++;
        } else if (strcmp(argv[i], "-w") == 0) {
            okno(args, atoi(args->arr[i + 1]));
            i++;
        } else if (strcmp(argv[i], "-f") == 0) {
            st = atoi(args->arr[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-e") == 0) {
            fin = atoi(args->arr[i + 1]);
            i++;
        } else {
            int raw = 0;
            for (int x = 0; x < strlen(args->arr[i]); x++) {
                if (raw) {
                    chislo = chislo + args->arr[i][x];
                } else {
                    nazvanie = nazvanie + args->arr[i][x];
                }
                if (args->arr[i][x] == '=')
                raw = 1;
            }
            if (nazvanie == "--output=") {
                file_path(args, chislo);
            } else if (nazvanie == "--stats=") {
                stats(args, stoi(chislo));
            } else if (nazvanie == "--window=") {
                okno(args, stoi(chislo));
            } else if (nazvanie == "--from=") {
                st = stoi(chislo);
            } else if (nazvanie == "--to=") {
                fin = stoi(chislo);
            } else {
                return false;
            }
 
        }
 
    }
}
 
 
int main(int argc, char *argv[]) {
    TArgs args;
    for (int i = 0; i < argc; ++i) {
        args.arr[i] = argv[i];
    }
    if (strcmp(argv[argc - 1], "access_log.txt") != 0) {
        printf("error");
        return 1;
    }
 
    //AnalyzeLog -f 66 -e 778777878878 -s 1000 access_log.txt
    //AnalyzeLog -w 1000 access_log.txt
    //a AnalyzeLog --stats=10000 access_log.txt
    if (!ReadArgs(&args, argc, argv)) {
 
        return 1;
    }
 
   return 0;
}