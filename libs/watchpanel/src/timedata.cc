#include "timedata.h"

#include "rapidjson/pointer.h"

#include <ctime>

namespace wpp = watchpanel; 

const char *words_hours12[12] = { 
    "Twelve",
    "One",
    "Two",
    "Three",
    "Four",
    "Five",
    "Six",
    "Seven",
    "Eight",
    "Nine",
    "Ten",
    "Eleven"
};

const char *words_minutes20[20] = { 
    "O\' Clock",
    "O\' One",
    "O\' Two",
    "O\' Three",
    "O\' Four",
    "O\' Five",
    "O\' Six",
    "O\' Seven",
    "O\' Eight",
    "O\' Nine",
    "Ten",
    "Eleven",
    "Twelve",
    "Thirteen",
    "Fourteen",
    "Fifteen",
    "Sixteen",
    "Seventeen",
    "Eighteen",
    "Nineteen"
};

const char *words_tens[12] = { 
    "Twenty",
    "Thirty",
    "Forty",
    "Fifty"
};

wpp::TimeData::TimeData() : DataImport("time") {}

wpp::TimeData::~TimeData() {}

void wpp::TimeData::Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds) {
    (void)deltaSeconds;
    std::time_t t = static_cast<std::time_t>(now);
    std::tm* localNow = std::localtime(&t);
    char value[16];
    std::strftime(value, 3, "%H", localNow);
    SetValueByPointer(out, "/hh", value);
    std::strftime(value, 3, "%M", localNow);
    SetValueByPointer(out, "/MM", value);

    int hour = localNow->tm_hour;
    bool pm = hour > 11;
    if (pm) {
        hour = hour - 12;
    }    int minute = localNow->tm_min;

    SetValueByPointer(out, "/whour", words_hours12[hour]);

    if (minute < 20) {
        SetValueByPointer(out, "/wmins", words_minutes20[minute]);
    } else {
        int tens = (minute / 10) - 2;
        SetValueByPointer(out, "/wtens", words_tens[tens]);
        int ones = minute % 10;
        if (ones > 0) {
            SetValueByPointer(out, "/wmins", words_hours12[ones]);
        }
    }

}