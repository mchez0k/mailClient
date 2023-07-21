#ifndef AUTHENTICATOR_HPP
#define AUTHENTICATOR_HPP

class interactiveAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {
   const std::vector <vmime::shared_ptr <vmime::security::sasl::SASLMechanism> >
        getAcceptableMechanisms(
            const std::vector <vmime::shared_ptr <vmime::security::sasl::SASLMechanism> >& available,
            const vmime::shared_ptr <vmime::security::sasl::SASLMechanism>& suggested
        ) const {

        std::cout << std::endl << "Доступные SASL механизмы:" << std::endl;

        for (unsigned int i = 0 ; i < available.size() ; ++i) {

            std::cout << "  " << available[i]->getName();

            if (suggested && available[i]->getName() == suggested->getName()) {
                std::cout << "(предложенный)";
            }
        }

        std::cout << std::endl << std::endl;
        return defaultSASLAuthenticator::getAcceptableMechanisms(available, suggested);
    }

    void setSASLMechanism(const vmime::shared_ptr <vmime::security::sasl::SASLMechanism>& mech) {

        std::cout << "Подключение к " << mech->getName() << std::endl;

        defaultSASLAuthenticator::setSASLMechanism(mech);
    }

    const vmime::string getUsername() const {

        if (m_username.empty()) {
            m_username = getUserInput("Username");
        }

        return m_username;
    }

    const vmime::string getPassword() const {

        if (m_password.empty()) {
            m_password = getUserInput("Password");
        }

        return m_password;
    }

    static const vmime::string getUserInput(const std::string& prompt) {

        std::cout << prompt << ": ";
        std::cout.flush();

        vmime::string res;
        std::getline(std::cin, res);

        return res;
    }

    private:

        mutable vmime::string m_username;
        mutable vmime::string m_password;
};
#endif //AUTHENTICATOR_HPP
