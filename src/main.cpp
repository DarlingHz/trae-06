#include <iostream>
#include <string>
#include <httplib.h>
#include "controller/survey_controller.h"

using namespace httplib;
using namespace controller;

int main(int argc, char* argv[]) {
    // Initialize survey controller
    SurveyController survey_controller;
    std::string db_path = "data/survey.db";

    if (!survey_controller.init(db_path)) {
        std::cerr << "Failed to initialize survey controller" << std::endl;
        return 1;
    }

    // Create HTTP server
    Server svr;
    int port = 8080;

    // Set up routes

    // 1. Create survey
    svr.Post("/api/surveys", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[POST] /api/surveys" << std::endl;

        std::string response = survey_controller.handleCreateSurvey(req.body);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 2. Publish survey
    svr.Post("/api/surveys/(\\w+)/publish", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[POST] /api/surveys/" << survey_id << "/publish" << std::endl;

        std::string response = survey_controller.handlePublishSurvey(survey_id);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 3. Close survey
    svr.Post("/api/surveys/(\\w+)/close", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[POST] /api/surveys/" << survey_id << "/close" << std::endl;

        std::string response = survey_controller.handleCloseSurvey(survey_id);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 4. Get survey by ID
    svr.Get("/api/surveys/(\\w+)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[GET] /api/surveys/" << survey_id << std::endl;

        std::string response = survey_controller.handleGetSurveyById(survey_id);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 5. Get surveys by owner ID
    svr.Get("/api/surveys", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[GET] /api/surveys" << std::endl;

        std::string response = survey_controller.handleGetSurveysByOwnerId(req.params);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 6. Add questions to survey
    svr.Post("/api/surveys/(\\w+)/questions", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[POST] /api/surveys/" << survey_id << "/questions" << std::endl;

        std::string response = survey_controller.handleAddQuestionsToSurvey(survey_id, req.body);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 7. Submit response
    svr.Post("/api/surveys/(\\w+)/responses", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[POST] /api/surveys/" << survey_id << "/responses" << std::endl;

        std::string response = survey_controller.handleSubmitResponse(survey_id, req.body);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // 8. Get survey stats
    svr.Get("/api/surveys/(\\w+)/stats", [&](const httplib::Request& req, httplib::Response& res) {
        std::string survey_id = req.matches[1];
        std::cout << "[GET] /api/surveys/" << survey_id << "/stats" << std::endl;

        std::string response = survey_controller.handleGetSurveyStats(survey_id);
        res.set_content(response, "application/json");
        res.status = 200;
    });

    // Start server
    std::cout << "Starting survey server on port " << port << "..." << std::endl;
    std::cout << "Database path: " << db_path << std::endl;

    if (svr.listen("0.0.0.0", port)) {
        std::cout << "Server started successfully!" << std::endl;
    } else {
        std::cerr << "Failed to start server on port " << port << std::endl;
        survey_controller.close();
        return 1;
    }

    // Close resources when server stops
    survey_controller.close();

    return 0;
}
