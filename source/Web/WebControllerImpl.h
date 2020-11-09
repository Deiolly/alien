#pragma once

#include "WebController.h"

class WebControllerImpl : public WebController
{
public:
    WebControllerImpl();
    virtual ~WebControllerImpl() = default;

    void requestSimulationInfos() override;
    void requestConnectToSimulation(int simulationId, std::string const& password) override;
    void requestTask(std::string const& simulationId) override;
    void requestDisconnect(std::string const& simulationId) override;

private:
    Q_SLOT void dataReceived(int handler, QByteArray data);

private:

    HttpClient* _http = nullptr;

    enum class RequestType {
        SimulationInfo,
        Connect
    };
    set<RequestType> _requesting;
};