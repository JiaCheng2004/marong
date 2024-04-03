#include <marong/exp.h>

void addExp(nlohmann::json& user, int amount) {
    int exp = user["status"]["exp"];
    int lvl = user["status"]["level"];

    if (exp >= 500) {
        
    }
}

void loseExp(nlohmann::json& user, int amount) {

}