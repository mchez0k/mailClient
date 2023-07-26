#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "authenticator.hpp"
#include "example6_certificateVerifier.hpp"
#include "example6_timeoutHandler.hpp"

static vmime::shared_ptr <vmime::net::session> g_session = vmime::net::session::create();

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

static void entry(GtkWidget* button, GtkWidget *loginInput){

    GtkEntry *loginEntry = GTK_ENTRY(loginInput);
    gchar *pass1;
    pass1 = (gchar*)(g_object_get_data(G_OBJECT(loginInput), "password2"));
    std::cout << gtk_entry_get_text(loginEntry) << std::endl;
    std::cout << pass1 << std::endl;
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
        //std::cerr << std::endl;
        //std::cerr << e << std::endl;
        throw;
    }
}

GtkWidget* logPassInput() // Поля ввода логина и пароля
{
    GtkWidget *vbox, *hboxLog, *hboxPass, *question, *loginLabel, *passwordLabel, *loginInput, *passwordInput, *buttonEntry;
    gchar *strLog, *password;
    strLog = g_strconcat ("Введите почту и пароль", NULL);
    question = gtk_label_new (strLog);
    loginLabel = gtk_label_new ("Email:");
    passwordLabel = gtk_label_new("Пароль:");

    loginInput = gtk_entry_new ();

    passwordInput = gtk_entry_new ();

    password = "pass";
    g_object_set_data(G_OBJECT(loginInput), "password2", password);

    gtk_entry_set_visibility (GTK_ENTRY (passwordInput), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (passwordInput), '*');

    hboxLog = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (hboxLog), loginLabel);
    gtk_box_pack_start_defaults (GTK_BOX (hboxLog), loginInput);

    hboxPass = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (hboxPass), passwordLabel);
    gtk_box_pack_start_defaults (GTK_BOX (hboxPass), passwordInput);

    buttonEntry = gtk_button_new_with_label ("Войти");

    g_signal_connect (G_OBJECT (buttonEntry), "clicked", G_CALLBACK (entry), loginInput);

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), question);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hboxLog);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hboxPass);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), buttonEntry);

    return vbox;
}

void initWindow(int argc, char *argv[]) // Инициализация главного окна
{
    GtkWidget *window, *vbox;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Вход для NPO MASH");
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);

    vbox = logPassInput();

    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
    /*try {
        std::locale::global(std::locale(""));
    } catch (std::exception &) {
        std::setlocale(LC_ALL, "");
    }*/

    initWindow(argc, argv);

    gtk_main ();


//    for (bool quit = false; !quit; ){
//        quit = entry();
//    }
    return 0;
}
