#include <crow.h>
#include <iostream>
#include <string>
#include "utils/Database.hpp"
#include "daos/UserDAO.hpp"
#include "daos/AlbumDAO.hpp"
#include "daos/ImageItemDAO.hpp"
#include "daos/TagDAO.hpp"
#include "daos/ImageTagDAO.hpp"
#include "daos/ImageLikeDAO.hpp"
#include "controllers/AuthController.hpp"
#include "controllers/AlbumController.hpp"
#include "controllers/ImageController.hpp"

#include "utils/DatabaseInitializer.hpp"

int main() {
    try {

        std::cout << "Starting Image Collection and Album Management Service" << std::endl;

        // Initialize database connection
        std::cout << "Initializing database connection" << std::endl;
        utils::Database db("image_collection.db");
        if (!db.isOpen()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return 1;
        }

        // Initialize database tables
        std::cout << "Initializing database tables" << std::endl;
        utils::DatabaseInitializer db_initializer(db);
        if (!db_initializer.initialize()) {
            std::cerr << "Failed to initialize database tables" << std::endl;
            return 1;
        }

        // Initialize DAOs
        std::cout << "Initializing DAOs" << std::endl;
        daos::UserDAO user_dao(db);
        daos::AlbumDAO album_dao(db);
        daos::ImageItemDAO image_item_dao(db);
        daos::TagDAO tag_dao(db);
        daos::ImageTagDAO image_tag_dao(db);
        daos::ImageLikeDAO image_like_dao(db);

        // Initialize controllers
        std::cout << "Initializing controllers" << std::endl;
        controllers::AuthController auth_controller(user_dao);
        controllers::AlbumController album_controller(album_dao, image_item_dao);
        controllers::ImageController image_controller(image_item_dao, album_dao, tag_dao, image_tag_dao, image_like_dao, user_dao);

        // Initialize Crow HTTP server
        std::cout << "Initializing HTTP server" << std::endl;
        crow::App<> app;

        // Configure CORS

        // Auth routes
        CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::POST)
        ([&auth_controller](const crow::request& req) {
            return auth_controller.registerUser(req);
        });

        CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)
        ([&auth_controller](const crow::request& req) {
            return auth_controller.loginUser(req);
        });

        // Album routes
        CROW_ROUTE(app, "/api/albums").methods(crow::HTTPMethod::POST)
        ([&album_controller](const crow::request& req) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return album_controller.createAlbum(req, token);
        });

        CROW_ROUTE(app, "/api/albums/mine").methods(crow::HTTPMethod::GET)
        ([&album_controller](const crow::request& req) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return album_controller.getMyAlbums(req, token);
        });

        CROW_ROUTE(app, "/api/albums/<int>").methods(crow::HTTPMethod::GET)
        ([&album_controller](const crow::request& req, int album_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return album_controller.getAlbumById(req, token, album_id);
        });

        CROW_ROUTE(app, "/api/albums/<int>").methods(crow::HTTPMethod::PUT)
        ([&album_controller](const crow::request& req, int album_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return album_controller.updateAlbum(req, token, album_id);
        });

        CROW_ROUTE(app, "/api/albums/<int>").methods(crow::HTTPMethod::DELETE)
        ([&album_controller](const crow::request& req, int album_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return album_controller.deleteAlbum(req, token, album_id);
        });

        // Image routes
        CROW_ROUTE(app, "/api/albums/<int>/images").methods(crow::HTTPMethod::POST)
        ([&image_controller](const crow::request& req, int album_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.addImageToAlbum(req, token, album_id);
        });

        CROW_ROUTE(app, "/api/albums/<int>/images").methods(crow::HTTPMethod::GET)
        ([&image_controller](const crow::request& req, int album_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.getImagesInAlbum(req, token, album_id);
        });

        CROW_ROUTE(app, "/api/images/<int>").methods(crow::HTTPMethod::PUT)
        ([&image_controller](const crow::request& req, int image_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.updateImage(req, token, image_id);
        });

        CROW_ROUTE(app, "/api/images/<int>").methods(crow::HTTPMethod::DELETE)
        ([&image_controller](const crow::request& req, int image_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.deleteImage(req, token, image_id);
        });

        CROW_ROUTE(app, "/api/images/search").methods(crow::HTTPMethod::GET)
        ([&image_controller](const crow::request& req) {
            return image_controller.searchPublicImages(req);
        });

        CROW_ROUTE(app, "/api/images/popular").methods(crow::HTTPMethod::GET)
        ([&image_controller](const crow::request& req) {
            return image_controller.getPopularPublicImages(req);
        });

        CROW_ROUTE(app, "/api/images/<int>/like").methods(crow::HTTPMethod::POST)
        ([&image_controller](const crow::request& req, int image_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.likeImage(req, token, image_id);
        });

        CROW_ROUTE(app, "/api/images/<int>/like").methods(crow::HTTPMethod::DELETE)
        ([&image_controller](const crow::request& req, int image_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.unlikeImage(req, token, image_id);
        });

        CROW_ROUTE(app, "/api/images/<int>/likes").methods(crow::HTTPMethod::GET)
        ([&image_controller](const crow::request& req, int image_id) {
            // Get token from Authorization header
            std::string token;
            auto auth_header_it = req.headers.find("Authorization");
            if (auth_header_it != req.headers.end()) {
                std::string auth_header = auth_header_it->second;
                if (auth_header.substr(0, 7) == "Bearer ") {
                    token = auth_header.substr(7);
                }
            }
            return image_controller.getImageLikes(req, token, image_id);
        });

        // Health check route
        CROW_ROUTE(app, "/health").methods(crow::HTTPMethod::GET)
        ([]() {
            crow::json::wvalue response;
            response["status"] = "ok";
            response["message"] = "Image Collection and Album Management Service is running";
            return crow::response(200, response);
        });

        // Start the server
        int port = 8080;
        std::cout << "Starting server on port " << port << std::endl;
        app.port(port).run();

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
