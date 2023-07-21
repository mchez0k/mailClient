#include <iostream>
#include <vector>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "authenticator.hpp"
#include "example6_certificateVerifier.hpp"
#include "example6_timeoutHandler.hpp"

static vmime::shared_ptr <vmime::net::session> g_session = vmime::net::session::create();

using namespace std;

static const std::string findAvailableProtocols(const vmime::net::service::Type type) {

    vmime::shared_ptr <vmime::net::serviceFactory> sf =
        vmime::net::serviceFactory::getInstance();

    std::ostringstream res;
    size_t count = 0;

    for (size_t i = 0 ; i < sf->getServiceCount() ; ++i) {

        const vmime::net::serviceFactory::registeredService& serv = *sf->getServiceAt(i);

        if (serv.getType() == type) {

            if (count != 0) {
                res << ", ";
            }

            res << serv.getName();
            ++count;
        }
    }

    return res.str();
}

static bool entry(){
    try {
        vmime::string urlString = "imaps://vpk.npomash.ru:993";
        vmime::utility::url url(urlString);

        std::cout << "Вы подключаетесь к " << urlString << std::endl;

        vmime::shared_ptr <vmime::net::store> st;

        st = g_session->getStore(url, vmime::make_shared <interactiveAuthenticator>());

        if (VMIME_HAVE_TLS_SUPPORT){
            st->setProperty("connection.tls", true);
            st->setTimeoutHandlerFactory(vmime::make_shared <timeoutHandlerFactory>());

            st->setCertificateVerifier(vmime::make_shared<interactiveCertificateVerifier>());
        }

        st->connect();

        vmime::shared_ptr <vmime::net::connectionInfos> ci = st->getConnectionInfos();

        std::cout << std::endl;
        std::cout << "Соединение с " << ci->getHost() << ci->getPort() << std::endl;
        std::cout << "Соединение " << (st->isSecuredConnection() ? "" : "не ") << "безопасно." << std::endl;

    } catch (vmime::exception& e) {
    //    std::cerr << std::endl;
      //  std::cerr << e << std::endl;
        throw;
        return false;
    }
    return false;
}

int main()
{
    try {
        std::locale::global(std::locale(""));
    } catch (std::exception &) {
        std::setlocale(LC_ALL, "");
    }

    for (bool quit = false; !quit; ){
        quit = entry();
    }

    return 0;
}
