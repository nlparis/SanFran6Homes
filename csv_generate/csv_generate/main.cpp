//
//  main.cpp
//  csv_generate
//
//  Created by Nick Paris on 10/26/17.
//  Copyright © 2017 Nick Paris. All rights reserved.
//
//This code divides up the San Francisco area into a 100x100 grid based
//on the min/max longitude and latitudes from listings.csv.
//It then uses the weekly price data calculated in weekly_price.csv (my own created file)
//to compute an average price for each location.
//If a location has no price (such as the water or other 'remote' areas,
//the algorithm searches its nearest neighbor until it reaches a price > 10
//10 is the lowest non-zero listing price from listings.csv
//Note that this means you could have a listing in the water and get paid for it....
//this is unrealistic, but avoiding the water would be much harder.
//The advantage is that all land is covered within the longitude/latitude
//ranges from listings.csv.

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "csvfile.h"
#include <iomanip>

using namespace std;

int main(int argc, const char * argv[]) {
#ifdef __APPLE__
    freopen("/Users/nickparis/SanFran6Homes/weekly_price.csv", "r", stdin);
#endif
   
    struct location{
        int price;
        double longitude, latitude;
        int bucket_num_long;
        int bucket_num_lat;
    };
    
    vector<location> homes;
    homes.reserve(8707);
    
    string line;
    getline(cin, line);
    
    double longitude = 0, latitude = 0;
    int price;
    string token;
    double min_longitude = 0;
    double max_longitude = 0;
    double min_latitude = 0;
    double max_latitude = 0;
    
    
    while (getline(cin, line)){
        line.erase(0, line.find(',') + 1);
        token = line.substr(0, line.find(','));
        latitude = atof(token.c_str());
        line.erase(0, line.find(',') + 1);
        token = line.substr(0, line.find(','));
        longitude = atof(token.c_str());
        line.erase(0, line.find(',') + 1);
        line.erase(0, line.find(',') + 1);
        line.erase(0, line.find(',') + 1);
        line.erase(0, line.find(',') + 1);
        token = line.substr(0, line.length());
        price = atoi(token.c_str());
        
        if (latitude < min_latitude || min_latitude == 0){
            min_latitude = latitude;
        }
        if (latitude > max_latitude || max_latitude == 0){
            max_latitude = latitude;
        }
        if (longitude < min_longitude || min_longitude == 0){
            min_longitude = longitude;
        }
        if (longitude > max_longitude || max_longitude == 0){
            max_longitude = longitude;
        }
        
        location add;
        add.price = price;
        add.latitude = latitude;
        add.longitude = longitude;
        homes.push_back(add);
    }
    
    double latitude_range_step = (fabs(max_latitude) - fabs(min_latitude)) / 100.000;
    double longitude_range_step = (fabs(max_longitude) - fabs(min_longitude))  / 100.000;
    
    for (int i = 0; i < homes.size(); ++i){
        homes[i].latitude -= min_latitude;
        homes[i].longitude -= min_longitude;
        homes[i].latitude /= latitude_range_step;
        homes[i].longitude /= longitude_range_step;
        
        homes[i].bucket_num_lat = (int)homes[i].latitude;
        homes[i].bucket_num_long = (int)homes[i].latitude;
    }
    
    struct price_aggregate{
        int price = 0;
        int num_entries = 0;
    };
    vector<price_aggregate> prices;
    prices.resize(10000);
    
    for (int i = 0; i < homes.size(); ++i){
        prices[homes[i].bucket_num_lat*100.00000 + homes[i].bucket_num_long].price += homes[i].price;
        ++prices[homes[i].bucket_num_lat*100.00000  + homes[i].bucket_num_long].num_entries;
    }
    
    for (int i = 0; i < prices.size(); ++i){
        if (prices[i].num_entries > 0){
            prices[i].price /= prices[i].num_entries;
        }
    }
    
    for (int i = 0; i < prices.size(); ++i){
        int x = 100;
        int y = 1;
        while (prices[i].price == 0){
            prices[i].num_entries = 0;
            if (i - x >= 0){
                prices[i].price += prices[i - x].price;
                prices[i].num_entries++;
            }
            if (i + x < 9900){
                prices[i].price += prices[i + x].price;
                prices[i].num_entries++;
            }
            if (i - y >= 0){
                prices[i].price += prices[i - y].price;
                prices[i].num_entries++;
            }
            if (i + y % x != x){
                prices[i].price += prices[i + y].price;
                prices[i].num_entries++;
            }
            
            //10 is the minimum price found in listings.csv (that isnt 0)
            if (prices[i].price /  prices[i].num_entries < 10){
                prices[i].price = 0;
                prices[i].num_entries = 0;
                x += 100.00000;
                y++;
            } else {
                prices[i].price /= prices[i].num_entries;
                prices[i].price++;
            }
        }
    }
    
    try
    {
        csvfile csv("avg_prices.csv"); // throws exceptions!
        // Header
        csv <<  std::fixed << std::setprecision(7) << "latitude" << "longitude" << "price" << endrow;
        // Data
        for (int i = 0; i < prices.size(); ++i){
            csv << std::fixed << std::setprecision(7) << min_latitude << min_longitude << prices[i].price << endrow;
            min_latitude += latitude_range_step/100.00000;
            min_longitude -= longitude_range_step/100.0000;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception was thrown: " << e.what() << std::endl;
    }
    
    return 0;
}
