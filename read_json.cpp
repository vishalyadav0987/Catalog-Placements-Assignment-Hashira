#include <iostream>
#include <fstream>
#include "json.hpp" // make sure -I/path/to/json/include is correct
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;



#include "json.hpp" // Include nlohmann/json

using json = nlohmann::json;
using namespace std;

using ld=long double;
using Point = pair<ld,ld>;

// Convert Base-X string to Decimal
ld convertToDecimal(const string& value, int base) {
    ld result = 0;
    for (char c : value) {
        int digit;
        if (isdigit(c)) digit = c - '0';
        else digit = tolower(c) - 'a' + 10;
        result = result * base + digit;
    }
    return result;
}



vector<Point> GetJsonDataAsInput(const string& filename, int& k) {
    ifstream file(filename);
    json j;
    file >> j;

    k = j["keys"]["k"];
    vector<Point> points;

    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        int x = stoi(el.key());
        int base = stoi(el.value()["base"].get<string>());
        string val = el.value()["value"];
        ld y = convertToDecimal(val, base);
        points.emplace_back(x, y);
    }

    return points;
}

// Lagrange interpolation to get f(0) using k points

ld lagrangeInterpolation(const vector<Point>& pts) {
    ld secret = 0;
    int k = pts.size();

    for (int i = 0; i < k; ++i) {
        ld xi = pts[i].first;
        ld yi = pts[i].second;

        ld li = 1;
        for (int j = 0; j < k; ++j) {
            if (i != j) {
                ld xj = pts[j].first;
                li *= (0 - xj) / (xi - xj);
            }
        }
        secret += yi * li;
    }
    return secret;
}



// Recovery of the best secret from all combinations of k points
// This function generates all combinations of k points from allPoints
// and finds the one that gives the most frequent secret value
// It uses Lagrange interpolation to compute the secret from the selected points
// and keeps track of the most frequent secret value found
// It prints the best secret value for each file processed
// This function is called in the main function for each JSON file
void RecoveryOfBestSecret(const vector<Point>& allPoints, int k, const string& label) {
    int n = allPoints.size();
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);

    unordered_map<long long, int> freq;
    long long bestSecret = -1;
    int maxFreq = 0;

    vector<int> comb;
    function<void(int, int)> generate = [&](int start, int depth) {
        if (depth == k) {
            vector<Point> subset;
            for (int idx : comb) subset.push_back(allPoints[idx]);
            ld s = lagrangeInterpolation(subset);
            long long rounded = llround(s);
            freq[rounded]++;
            if (freq[rounded] > maxFreq) {
                maxFreq = freq[rounded];
                bestSecret = rounded;
            }
            return;
        }
        for (int i = start; i <= n - (k - depth); ++i) {
            comb.push_back(i);
            generate(i + 1, depth + 1);
            comb.pop_back();
        }
    };

    generate(0, 0);
    cout << "Best secret from " << label << ": " << bestSecret << endl;
}

int main() {
    vector<string> jsonData = {"sample.json", "sample2.json"};
    for (const string& json : jsonData) {
        int k;
        auto points = GetJsonDataAsInput(json, k);
        RecoveryOfBestSecret(points, k, json);
    }
    return 0;
}