#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <random>
#include <map>

#include <zmq.hpp>
#include <zmq_addon.hpp>

using namespace std;

void NetworkWindow() {
    static char ip[64] = "127.0.0.1";
    static int port = 5555;
    static char message[512] = "";
    static std::vector<std::string> messageHistory;
    static const int MAX_MESSAGES = 10;
    static zmq::context_t context(1);
    static zmq::socket_t client(context, ZMQ_REQ);
    static zmq::socket_t server(context, ZMQ_REP);
    static bool clientConnected = false;
    static bool serverBound = false;
    static bool waitingForReply = false;

    ImGui::Begin("Network Communication");

    // IP and Port input
    ImGui::Text("Target IP:");
    ImGui::InputText("##ip", ip, sizeof(ip));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("Port", &port);

    // Connection management
    if (ImGui::Button("Connect as Client")) {
        try {
            if (clientConnected) {
                client.close();
                client = zmq::socket_t(context, ZMQ_REQ);
            }
            std::string address = "tcp://" + std::string(ip) + ":" + std::to_string(port);
            client.connect(address);
            clientConnected = true;
            waitingForReply = false;
        }
        catch (const zmq::error_t& e) {
            messageHistory.push_back("Connection error: " + std::string(e.what()));
            if (messageHistory.size() > MAX_MESSAGES) {
                messageHistory.erase(messageHistory.begin());
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Start Server")) {
        try {
            if (serverBound) {
                server.close();
                server = zmq::socket_t(context, ZMQ_REP);
            }
            std::string address = "tcp://*:" + std::to_string(port);
            server.bind(address);
            server.set(zmq::sockopt::rcvtimeo, 10); // 10ms timeout
            serverBound = true;
        }
        catch (const zmq::error_t& e) {
            messageHistory.push_back("Bind error: " + std::string(e.what()));
            if (messageHistory.size() > MAX_MESSAGES) {
                messageHistory.erase(messageHistory.begin());
            }
        }
    }

    ImGui::Separator();

    // Message sending (client side)
    ImGui::Text("Send Message (Client):");
    ImGui::InputText("##message", message, sizeof(message));
    ImGui::SameLine();
    if (ImGui::Button("Send") && clientConnected && !waitingForReply) {
        try {
            zmq::message_t zmq_msg(strlen(message));
            memcpy(zmq_msg.data(), message, strlen(message));
            client.send(zmq_msg, zmq::send_flags::dontwait);
            waitingForReply = true;

            messageHistory.push_back("[SENT] " + std::string(message));
            if (messageHistory.size() > MAX_MESSAGES) {
                messageHistory.erase(messageHistory.begin());
            }
        }
        catch (const zmq::error_t& e) {
            messageHistory.push_back("Send error: " + std::string(e.what()));
            if (messageHistory.size() > MAX_MESSAGES) {
                messageHistory.erase(messageHistory.begin());
            }
        }
    }

    // Check for client reply (silent acknowledgment)
    if (waitingForReply && clientConnected) {
        try {
            zmq::message_t zmq_msg;
            auto result = client.recv(zmq_msg, zmq::recv_flags::dontwait);
            if (result) {
                waitingForReply = false; // Just reset the state, don't display the ack
            }
        }
        catch (const zmq::error_t& e) {
            // Ignore timeout errors
        }
    }

    ImGui::Separator();

    // Server receiving
    if (serverBound) {
        try {
            zmq::message_t zmq_msg;
            auto result = server.recv(zmq_msg, zmq::recv_flags::dontwait);
            if (result) {
                std::string received(static_cast<char*>(zmq_msg.data()), zmq_msg.size());

                // Get source info
                std::string sourceInfo = "unknown";
                try {
                    char endpoint[256];
                    size_t endpoint_size = sizeof(endpoint);
                    server.getsockopt(ZMQ_LAST_ENDPOINT, endpoint, &endpoint_size);
                    sourceInfo = std::string(endpoint);

                    size_t pos = sourceInfo.find("://");
                    if (pos != std::string::npos) {
                        sourceInfo = sourceInfo.substr(pos + 3);
                    }
                }
                catch (...) {
                    sourceInfo = "local";
                }

                messageHistory.push_back("[RECEIVED from " + sourceInfo + "] " + received);
                if (messageHistory.size() > MAX_MESSAGES) {
                    messageHistory.erase(messageHistory.begin());
                }

                // Send required acknowledgment (but don't display it)
                std::string ack = "ACK";
                zmq::message_t ack_msg(ack.size());
                memcpy(ack_msg.data(), ack.c_str(), ack.size());
                server.send(ack_msg, zmq::send_flags::dontwait);
            }
        }
        catch (const zmq::error_t& e) {
            // Ignore timeout errors
        }
    }

    ImGui::Separator();

    // Display message history
    ImGui::Text("Message History (max %d):", MAX_MESSAGES);
    ImGui::BeginChild("MessageHistory", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& msg : messageHistory) {
        ImGui::TextWrapped("%s", msg.c_str());
    }
    if (!messageHistory.empty()) {
        ImGui::SetScrollHereY(1.0f); // Auto-scroll to bottom
    }
    ImGui::EndChild();

    if (ImGui::Button("Clear History")) {
        messageHistory.clear();
    }

    ImGui::End();
}

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1280, 720 }), "My window");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        float deltaTime = deltaClock.getElapsedTime().asSeconds();

        // Event Polling
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Update
        ImGui::SFML::Update(window, deltaClock.restart());

        NetworkWindow();

        // Render
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

	return 0;
}