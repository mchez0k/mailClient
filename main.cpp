#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "authenticator.hpp"
#include "example6_certificateVerifier.hpp"
#include "example6_timeoutHandler.hpp"
/*
 * Весь код написан в функциональном стиле, так как содержит смесь C и C++
 * Ничего не мешает использовать части кода, следуя принципам ООП
 *
 * Информация по библиотеке GTK+ в книге Foundations of GTK+
 *
 * Информация по библиотеке VMIME находится в книге VmimeBook
 *
 */
enum
{
  FOLDERNAME,
  COUNT,
  UNSEEN,
  COLUMNS
};

enum
{
    FOLDER,
    MESSAGE
};

vmime::shared_ptr <vmime::net::session> g_session = vmime::net::session::create(); // Текущая сессия
vmime::shared_ptr <vmime::net::store> st; // Хранилище (imap store)

GtkWidget *loginWindow, *trayMenu, *trayMenuItemView, *trayMenuItemExit, *windowTree;
GtkStatusIcon *trayIcon;

std::vector<vmime::shared_ptr<vmime::net::folder>> treeFolders; // Папки для TreeView

static void setup_tree_view (GtkWidget *treeview, bool isMain = true)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  vmime::size_t count = 0, unseen = 0; // Кол-во непросмотренных писем всего и непросмотренных писем в клиенте

  renderer = gtk_cell_renderer_text_new ();
  if (isMain){
      column = gtk_tree_view_column_new_with_attributes
                             ("Название папки", renderer, "text", FOLDERNAME, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      column = gtk_tree_view_column_new_with_attributes
                             ("Всего сообщений", renderer, "text", COUNT, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      column = gtk_tree_view_column_new_with_attributes
                             ("Непрочитанных", renderer, "text", UNSEEN, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  }
}

static void trayView(GtkMenuItem *item, gpointer window)
{
    gtk_widget_show(GTK_WIDGET(window));
    gtk_window_deiconify(GTK_WINDOW(window));
}

static void trayExit(GtkMenuItem *item, gpointer user_data)
{
    printf("exit");
    gtk_main_quit();
}


static void trayIconActivated(GObject *trayIcon, gpointer window)
{
    gtk_widget_show(GTK_WIDGET(window));
    gtk_window_deiconify(GTK_WINDOW(window));
}

static void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu)
{
    gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}


static gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event, gpointer trayIcon) // Проверка открытого окна в трее
{
    if(event->changed_mask == GDK_WINDOW_STATE_ICONIFIED && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
    {
        gtk_widget_hide (GTK_WIDGET(widget));
        gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), TRUE);
    }
    else if(event->changed_mask == GDK_WINDOW_STATE_WITHDRAWN && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
    {
        gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), FALSE);
    }
    return TRUE;
}

static const std::string findAvailableProtocols(const vmime::net::service::Type type)
{

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

static void addFolders(vmime::shared_ptr <vmime::net::folder> folder) { // Функция для добавления папок в вектор папок

    treeFolders.push_back(folder);

    std::vector <vmime::shared_ptr <vmime::net::folder> > subFolders = folder->getFolders(false);

    for (unsigned int i = 0 ; i < subFolders.size() ; ++i) {
        addFolders(subFolders[i]);
    }
}

static const vmime::string getFolderPathString(vmime::shared_ptr <vmime::net::folder> f) { // Получить путь папки в виде строки

    const vmime::string n = f->getName().getBuffer();

    if (n.empty()) {

        return "/";

    } else {
        vmime::shared_ptr <vmime::net::folder> p = f->getParent();
        return getFolderPathString(p) + n + "/";
    }
}

static void messages(){ // Основное окно сообщений
    GtkWidget *treeview, *scrolled_win;
    GtkTreeStore *store;
    GtkTreeIter iter;

    windowTree = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (windowTree), "Messages");
    gtk_container_set_border_width (GTK_CONTAINER (windowTree), 10);
    gtk_widget_set_size_request (windowTree, 275, 300);

    g_signal_connect (G_OBJECT (windowTree), "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    treeview = gtk_tree_view_new ();
    setup_tree_view(treeview);

    vmime::shared_ptr <vmime::net::folder> root = st->getRootFolder();
    addFolders(root);

//    for (size_t i = 0; i != treeFolders.size(); i++) { Необходимо получить названия папок и вывести их через name в столбец FOLDERNAME
//        gtk_tree_store_append(store, &iter, NULL);
//        gtk_tree_store_set(store, &iter, FOLDERNAME, name, COUNT, 2, UNSEEN, 2, -1);
//    }
    store = gtk_tree_store_new (COLUMNS, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));
    gtk_tree_view_expand_all (GTK_TREE_VIEW (treeview));
    g_object_unref (store);

    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (scrolled_win), treeview);
    gtk_container_add (GTK_CONTAINER (windowTree), scrolled_win);

    trayMenu = gtk_menu_new();
    trayMenuItemView = gtk_menu_item_new_with_label ("View");
    trayMenuItemExit = gtk_menu_item_new_with_label ("Exit");
    g_signal_connect (G_OBJECT (trayMenuItemView), "activate", G_CALLBACK (trayView), windowTree);
    g_signal_connect (G_OBJECT (trayMenuItemExit), "activate", G_CALLBACK (trayExit), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (trayMenu), trayMenuItemView);
    gtk_menu_shell_append (GTK_MENU_SHELL (trayMenu), trayMenuItemExit);
    gtk_widget_show_all (trayMenu);

    trayIcon = gtk_status_icon_new_from_file ("/home/en4my/code/mailClient_v_0_1_0/logo.png");

    gtk_status_icon_set_tooltip (trayIcon, "Соединение установлено");
    g_signal_connect(GTK_STATUS_ICON (trayIcon), "activate", GTK_SIGNAL_FUNC (trayIconActivated), windowTree);
    g_signal_connect(GTK_STATUS_ICON (trayIcon), "popup-menu", GTK_SIGNAL_FUNC (trayIconPopup), trayMenu);
    g_signal_connect (G_OBJECT (loginWindow), "window-state-event", G_CALLBACK (window_state_event), trayIcon);

    gtk_widget_show_all (windowTree);
}

static void entry(GtkWidget* button, GtkObject *authData){ // Окно входа

    GtkEntry *loginEntry = GTK_ENTRY(gtk_object_get_data(authData, "login"));
    GtkEntry *passwordEntry = GTK_ENTRY(gtk_object_get_data(authData, "password"));
    gchar *login1, *pass1;
    login1 = (gchar*)gtk_entry_get_text(loginEntry);
    pass1 = (gchar*)gtk_entry_get_text(passwordEntry);
    try {
        vmime::string urlString = "imaps://imap.mail.ru:993"; //vpk.npomash.ru:993 imap.mail.ru:993
        vmime::utility::url url(urlString);
        url.setUsername(login1); // root.root.2023@mail.ru
        url.setPassword(pass1); // KSVrCfBk9hncemY2xyDp

        std::cout << "Вы подключаетесь к " << urlString << std::endl; // Вывод в консоль о статусе подключения, можно будет добавить в окна GTK

        st = g_session->getStore(url);

        if (VMIME_HAVE_TLS_SUPPORT){
            st->setProperty("connection.tls", true);
            st->setTimeoutHandlerFactory(vmime::make_shared <timeoutHandlerFactory>());

            st->setCertificateVerifier(vmime::make_shared<interactiveCertificateVerifier>());
        }
        std::cout << "Успешный tls" << std::endl;

        st->connect();

        std::cout << "Успешно" << std::endl;
        vmime::shared_ptr <vmime::net::connectionInfos> ci = st->getConnectionInfos();

        std::cout << std::endl;
        std::cout << "Соединение с " << ci->getHost() << ci->getPort() << std::endl;
        std::cout << "Соединение " << (st->isSecuredConnection() ? "" : "не ") << "безопасно." << std::endl;
        gtk_widget_hide(loginWindow);
        gtk_status_icon_set_tooltip (trayIcon, "Соединение установлено");
        messages();


    } catch (vmime::exception& e) {
        //std::cerr << std::endl;
        //std::cerr << e << std::endl;
        throw;
    }
}

GtkWidget* logPassInput() // Поля ввода логина и пароля
{
    GtkWidget *vbox, *hboxLog, *hboxPass, *question, *loginLabel, *passwordLabel,
            *loginInput, *passwordInput, *buttonEntry;
    gchar *strLog;
    strLog = g_strconcat ("Введите почту и пароль", NULL);
    question = gtk_label_new (strLog);
    loginLabel = gtk_label_new ("Email:");
    passwordLabel = gtk_label_new("Пароль:");

    loginInput = gtk_entry_new ();

    passwordInput = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (passwordInput), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (passwordInput), '*');

    gtk_object_set_data(GTK_OBJECT(loginInput), "login", loginInput);
    gtk_object_set_data(GTK_OBJECT(loginInput), "password", passwordInput);

    hboxLog = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (hboxLog), loginLabel);
    gtk_box_pack_start_defaults (GTK_BOX (hboxLog), loginInput);

    hboxPass = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (hboxPass), passwordLabel);
    gtk_box_pack_start_defaults (GTK_BOX (hboxPass), passwordInput);

    buttonEntry = gtk_button_new_with_label ("Войти");

    g_signal_connect (G_OBJECT (buttonEntry), "clicked", G_CALLBACK (entry), GTK_OBJECT(loginInput));

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), question);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hboxLog);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hboxPass);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), buttonEntry);

    return vbox;
}

void initWindow(int argc, char *argv[]) // Инициализация главного окна
{
    GtkWidget *vbox;

    gtk_init (&argc, &argv);

    loginWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (loginWindow), "Вход для NPO MASH");
    gtk_container_set_border_width (GTK_CONTAINER (loginWindow), 10);

    g_signal_connect (G_OBJECT (loginWindow), "destroy", gtk_main_quit, NULL);

    vbox = logPassInput();

    gtk_container_add (GTK_CONTAINER (loginWindow), vbox);
    gtk_widget_show_all (loginWindow);
}

int main(int argc, char *argv[])
{
    initWindow(argc, argv);

    gtk_main ();

    return 0;
}
