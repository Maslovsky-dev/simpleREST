#include "crow.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <filesystem>

// Глобальная переменная с путем к базе
const std::filesystem::path db_path = std::filesystem::current_path() / ".." / "messages.db";

void initDB()
{
    auto normalized_path = std::filesystem::weakly_canonical(db_path);
    std::cout << "Поиск файла бд. Путь: " << normalized_path << std::endl;

    if (!std::filesystem::exists(db_path))
    {
        throw std::runtime_error("файл бд не найден");
    }

    std::cout << "Файл бд найден" << std::endl;

    SQLite::Database db(db_path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::cout << "База открыта успешно" << std::endl;
    db.exec("CREATE TABLE IF NOT EXISTS user ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE)");
}

int main()
{
    initDB(); // Подготавливаем базу

    crow::SimpleApp app;

    // GET /users
    CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::GET)([]
    {
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> users;

        try
        {
            SQLite::Database db(db_path.string(), SQLite::OPEN_READONLY);
            SQLite::Statement query(db, "SELECT id, username FROM user");

            while (query.executeStep())
            {
                crow::json::wvalue user;
                user["id"] = query.getColumn(0).getInt();
                user["username"] = query.getColumn(1).getText();
                users.push_back(user);
            }
        }
        catch (const std::exception& e)
        {
            return crow::response(500, e.what());
        }

        result["users"] = std::move(users);
        return crow::response(result);
    });

    // POST /users
    CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::POST)([](const crow::request& req)
    {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username"))
        {
            return crow::response(400, "Missing username");
        }

        try
        {
            SQLite::Database db(db_path.string(), SQLite::OPEN_READWRITE);
            SQLite::Statement insert(db, "INSERT INTO user (username) VALUES (?)");
            insert.bind(1, body["username"].s());
            insert.exec();
        }
        catch (const std::exception& e)
        {
            return crow::response(500, e.what());
        }

        return crow::response(201, "User created");
    });

    // PUT /users/<int>
    CROW_ROUTE(app, "/users/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request& req, int id)
    {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username"))
        {
            return crow::response(400, "Missing username");
        }

        try
        {
            SQLite::Database db(db_path.string(), SQLite::OPEN_READWRITE);
            SQLite::Statement update(db, "UPDATE user SET username = ? WHERE id = ?");
            update.bind(1, body["username"].s());
            update.bind(2, id);
            update.exec();
        }
        catch (const std::exception& e)
        {
            return crow::response(500, e.what());
        }

        return crow::response(200, "User updated");
    });

    // DELETE /users/<int>
    CROW_ROUTE(app, "/users/<int>").methods(crow::HTTPMethod::DELETE)([](const crow::request& /*req*/, int id)
    {
        try
        {
            SQLite::Database db(db_path.string(), SQLite::OPEN_READWRITE);
            SQLite::Statement del(db, "DELETE FROM user WHERE id = ?");
            del.bind(1, id);
            del.exec();
        }
        catch (const std::exception& e)
        {
            return crow::response(500, e.what());
        }

        return crow::response(200, "User deleted");
    });

    std::cout << "Server running at http://localhost:18080" << std::endl;
    app.port(18080).multithreaded().run();
}
