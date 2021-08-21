#include <sstream>
#include "dump.hpp"

namespace knit
{
    namespace actor
    {
        namespace sink
        {
            namespace dump
            {
                Dump::Dump(const DumpParams &params) : params_(params), db_(nullptr), stmt_(nullptr), count_(0)
                {
                    int rc = sqlite3_open(params_.db.c_str(), &db_);
                    if (rc)
                    {
                        std::cout << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
                        throw Exception(ExceptionType::RUNTIME, "Can't open database");
                    }
                    else
                    {
                        std::cout << "Opened database successfully" << std::endl;
                    }

                    std::string sql = "INSERT INTO SENSORS_SPI_MESSAGE (NAME, TICK, LENGTH, DATA) values(?,?,?,?)";

                    rc = sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt_, NULL);
                    if (rc != SQLITE_OK)
                    {
                        std::cout << "sql error: " << sqlite3_errmsg(db_) << std::endl;
                        throw Exception(ExceptionType::RUNTIME, "sql error");
                    }
                    sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
                }
                Dump::~Dump()
                {
                    if (stmt_ != nullptr)
                    {
                        sqlite3_finalize(stmt_);
                    }

                    if (db_ != nullptr)
                    {
                        sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_close(db_);
                    }
                }
                void Dump::receive(Tube<SPIMessagePtr> &in)
                {
                    auto res = in.get();
                    const auto &name = res->name;
                    const auto &tick = res->tick;
                    const auto &length = res->length;
                    const auto &data = res->load;

                    std::stringstream ss;
                    ss << name;
                    std::string name_str;
                    ss >> name_str;

                    sqlite3_bind_text(stmt_, 1, name_str.c_str(), name_str.length(), SQLITE_STATIC);
                    sqlite3_bind_int(stmt_, 2, tick);
                    sqlite3_bind_int(stmt_, 3, length);
                    sqlite3_bind_blob(stmt_, 4, data.data(), data.size(), NULL);

                    int step_res = sqlite3_step(stmt_);
                    if (step_res != SQLITE_DONE)
                    {
                        throw Exception(ExceptionType::RUNTIME, "Insert failed. Errorcode");
                    }

                    sqlite3_reset(stmt_);

                    if (count_++ > 1000)
                    {
                        count_ = 0;
                        sqlite3_exec(db_, "END TRANSACTION;", NULL, NULL, NULL);
                        std::cout << "round a TRANSACTION" << std::endl;
                        sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
                    }
                }

            }
        }
    }
}