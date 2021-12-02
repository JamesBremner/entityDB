
#include "entitydb.h"

class cApp
{
public:
    cApp();
    void run()
    {
        myForm.run();
    }

private:
    wex::gui &myForm;
    raven::edb::cEntityForm myParentForm;
    raven::edb::cEntityForm myChildForm;

    void hide();
};

cApp::cApp()
    : myForm(wex::maker::make()),
      myParentForm(raven::edb::cEntityForm(
          myForm, "parent",
          {"First Name",
           "Last Name",
           "Email"})),
      myChildForm(raven::edb::cEntityForm(
          myForm, "child",
          {"First Name",
           "Last Name"}))
{
    raven::edb::open(
        "family.dat");

    raven::edb::createCategory(
        "parent",
        {"first", "last", "email"});

    raven::edb::createCategory(
        "child",
        {"first", "last", "parent"});

    myForm.move({50, 50, 600, 800});
    myForm.text("Family");

    wex::menubar mbar(myForm);
    wex::menu m(myForm);
    m.append("Parents",
             [&](const std::string &title)
             {
                 raven::edb::cEntityList L(
                     "Parents",
                     "parent");
                 hide();
                 myParentForm.show(L.mySelected);
             });
    m.append("Children",
             [&](const std::string &title)
             {
                 raven::edb::cEntityList L(
                     "Children",
                     "child");
                 hide();
                 myChildForm.show(L.mySelected);
             });
    mbar.append("Tables", m);

    myForm.show();
    hide();
}
void cApp::hide()
{
    myParentForm.show("-1", false);
    myChildForm.show("-1", false);
}

cApp *theApp;

main()
{
    theApp = new cApp();
    theApp->run();
}