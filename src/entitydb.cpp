#include <string>
#include <filesystem>
#include "wex.h"
#include "propertygrid.h"
#include "entitydb.h"

namespace raven
{
    namespace edb
    {
        cDB theDB;

        bool open(const std::string &fname)
        {
            return theDB.open(fname);
        }
        void createCategory(
            const std::string &name,
            const std::vector<std::string> &van)
        {
            theDB.createCategory(name, van);
        }

        bool cDB::open(const std::string &fname)
        {
            if (db.isOpen())
                return true;
            std::filesystem::path path(fname);
            if( ! path.parent_path().empty() )
                std::filesystem::create_directories(path.parent_path());
            db.Open(fname);
            if (!db.isOpen())
                return false;
            db.Query(
                "CREATE TABLE IF NOT EXISTS name_value "
                " ( id, category, name, value );");
            db.Query(
                "CREATE TABLE IF NOT EXISTS category "
                " ( id INTEGER PRIMARY KEY AUTOINCREMENT);");
            return true;
        }

        int cDB::createCategory(
            const std::string &name,
            const std::vector<std::string> &van)
        {
            if (!db.isOpen())
                throw std::runtime_error("DB not open");

            // create master table for new category
            db.Query(
                "CREATE TABLE IF NOT EXISTS %s "
                " ( id INTEGER PRIMARY KEY AUTOINCREMENT);",
                name.c_str());

            // check if category already present
            int ret = db.Query(
                "SELECT id FROM name_value "
                " WHERE category = '0' "
                " AND name = 'name' "
                " AND value = '%s'; ",
                name.c_str());
            if (ret == 1)
            {
                int catID = atoi(db.myResultA[0].c_str());
                categoryRead(catID, name);
                return catID;
            }

            // add category to master list
            if (0 > db.Query(
                        "INSERT INTO category DEFAULT VALUES;"))
                return -1;
            if (0 > db.Query(
                        "SELECT last_insert_rowid();"))
                return -1;
            auto rowid = db.myResultA[0];
            // write category attributes
            db.Query(
                "INSERT INTO name_value VALUES "
                " ( '%s', '0', 'name', '%s' ), "
                " ( '%s', '0', 'attcount', '%d' );",
                rowid.c_str(),
                name.c_str(),
                rowid.c_str(),
                (int)van.size());
            for (int k = 0; k < van.size(); k++)
            {
                std::string sn = "attname" + std::to_string(k);
                db.Query(
                    "INSERT INTO name_value VALUES "
                    " ( '%s', '0', '%s', '%s' );",
                    rowid.c_str(),
                    sn.c_str(),
                    van[k].c_str());
            }

            sCategory s;
            s.name = name;
            s.attname = van;
            myMapCategory[atoi(rowid.c_str())] = s;

            std::cout << "cDB::CreateCategory "
                      << myMapCategory.size()
                      << " " << name << "\n";
            return atoi(rowid.c_str());
        }

        void cDB::categoryRead(
            int catID,
            const std::string &catName)
        {
            sCategory s;
            s.name = catName;
            db.Query(
                "SELECT value FROM name_value "
                " WHERE id = '%d' "
                " AND category = '0' "
                " AND name = 'attcount';",
                catID);
            int attcount = atoi(db.myResultA[0].c_str());
            for (int k = 0; k < attcount; k++)
            {
                std::string sn = "attname" + std::to_string(k);
                db.Query(
                    "SELECT value FROM name_value "
                    " WHERE id = '%d' "
                    " AND category = '0' "
                    " AND name = '%s';",
                    catID,
                    sn.c_str());
                s.attname.push_back(db.myResultA[0]);
            }
            myMapCategory[catID] = s;
        }

        std::string cDB::add(
            int category,
            const std::vector<std::string> &nv)
        {
            if (!db.isOpen())
                return "-1";
            if (1 > category || category > myMapCategory.size())
                return "-1";
            if (0 > db.Query(
                        "INSERT INTO %s DEFAULT VALUES;",
                        myMapCategory[category].name.c_str()))
                return "-1";
            if (0 > db.Query(
                        "SELECT last_insert_rowid();"))
                return "-1";
            auto rowid = db.myResultA[0];
            for (int col = 0; col < nv.size() / 2; col++)
            {
                db.Query(
                    "INSERT INTO name_value VALUES "
                    " ( '%s', '%d', '%s', '%s' );",
                    rowid.c_str(),
                    category,
                    nv[2 * col].c_str(),
                    nv[2 * col + 1].c_str());
            }
            return rowid;
        }

        bool cDB::update(
            int category,
            const std::string &id,
            const std::vector<std::string> &nv)
        {
            if (id == "-1")
                throw std::runtime_error(
                    "cDB::update bad entity ID");

            for (int col = 0; col < nv.size() / 2; col++)
            {
                std::cout << nv[2 * col] << " " << nv[2 * col + 1] << "\n";
                if (0 > db.Query(
                            "UPDATE name_value "
                            " SET value = '%s' "
                            " WHERE id = '%s' "
                            " AND category = '%d' "
                            " AND name = '%s';",
                            nv[2 * col + 1].c_str(),
                            id.c_str(),
                            category,
                            nv[2 * col].c_str()))
                    return false;
                std::cout << db.queryformattedA << "\n";
            }
            return true;
        }

        void cDB::remove(
            int catID,
            const std::string &id)
        {
            db.Query(
                "DELETE FROM name_value "
                " WHERE id = '%s' "
                " AND category = '%d'; ",
                id.c_str(),
                catID);
            db.Query(
                " DELETE FROM '%s' WHERE rowid = '%s';",
                category(catID).c_str(),
                id.c_str());
        }

        std::vector<std::string> cDB::read(
            int category,
            const std::string &id)
        {
            db.Query(
                "SELECT value FROM name_value "
                "WHERE category = '%d' "
                "AND id = '%s'; ",
                category,
                id.c_str());
            return db.myResultA;
        }

        std::vector<std::vector<std::string>> cDB::list(
            int catID)
        {
            std::vector<std::vector<std::string>> ret;
            if (!db.isOpen())
            {
                std::cout << "cDB::list no db\n";
                return ret;
            }

            int count = db.Query(
                "SELECT rowid FROM %s; ",
                category(catID).c_str());
            if (count < 0)
            {
                std::cout << "DB error\n";
                std::cout << db.queryformattedA << "\n";
                return ret;
            }
            auto vpid = db.myResultA;

            // loop over entities
            for (auto &pid : vpid)
            {
                std::vector<std::string> row;
                row.push_back(pid);
                row.push_back(
                    text(catID, pid));
                ret.push_back(row);
            }
            return ret;
        }
        std::string cDB::text(
            int category,
            const std::string &pid)
        {
            std::string s;
            // loop over columns
            for (auto &col : read(category, pid))
                s += col + " ";
            return s;
        }

        cEntity::cEntity(
            const std::string &categoryName,
            const std::vector<std::string> &vat)
            : myCategory(theDB.category(categoryName)), myAt(vat)
        {
            if (myCategory == -1)
                throw std::runtime_error(
                    categoryName + " unknown category");
        }

        void cEntity::add()
        {
            myID = theDB.add(
                myCategory,
                nameValue());
        }
        void cEntity::update()
        {
            theDB.update(
                myCategory,
                myID,
                nameValue());
        }
        void cEntity::remove()
        {
            if (myID == "-1")
                return;
            theDB.remove(
                myCategory,
                myID);
        }
        void cEntity::read()
        {
            if (myID == "-1")
            {
                myAt.clear();
                for (
                    int col = 0;
                    col < theDB.categoryAttCount(myCategory);
                    col++)
                    myAt.push_back("");
            }
            else
            {
                myAt = theDB.read(myCategory, myID);
                if (myAt.size() != theDB.categoryAttCount(myCategory))
                {
                    std::cout
                        << "entID " << myID
                        << ", cat att count " << theDB.categoryAttCount(myCategory)
                        << ", read att count " << myAt.size() << "\n";
                    throw std::runtime_error("cEntity::read attribute count error");
                }
            }
        }
        std::vector<std::string> cEntity::nameValue() const
        {
            if (theDB.categoryAttCount(myCategory) != myAt.size())
                throw std::runtime_error("name value mismatch");
            std::vector<std::string> ret;
            for (
                int k = 0;
                k < theDB.categoryAttCount(myCategory);
                k++)
            {
                ret.push_back(theDB.categoryAttName(myCategory, k));
                ret.push_back(myAt[k]);
            }
            return ret;
        }
        std::string cEntity::text() const
        {
            std::string ret;
            for (auto &s : myAt)
                ret += s + '|';
            return ret;
        }

        int cDB::category(const std::string &name) const
        {
            auto it = myMapCategory.begin();
            for (;
                 it != myMapCategory.end();
                 it++)
            {
                if (it->second.name == name)
                    break;
            }
            if (it == myMapCategory.end())
                return -1;
            return it->first;
        }
        std::string cDB::category(int catID) const
        {
            return myMapCategory.at(catID).name;
        }
        int cDB::categoryAttCount(int catID) const
        {
            return myMapCategory.at(catID).attname.size();
        }
        std::string cDB::categoryAttName(int catID, int att) const
        {
            auto vn = myMapCategory.at(catID).attname;
            if (0 > att || att >= vn.size())
                throw std::runtime_error(
                    "cDB:categoryAttNam bad attribute index");
            return vn[att];
        }

        cEntityList::cEntityList(
            const std::string &title,
            const std::string &catName)
            : myForm(wex::maker::make()),
              myPG(wex::maker::make<wex::propertyGrid>(myForm)),
              mySelected("-1")
        {
            auto list = theDB.list(theDB.category(catName));
            if (!list.size())
                return;
            myForm.move({600, 100, 600, 500});
            myForm.text(title);
            myPG.move({10, 10, 550, 490});

            for (auto &row : list)
                myPG.string(row[0], row[1]);
            myPG.nameClick(
                [this](const std::string &pid)
                {
                    mySelected = pid;
                    myForm.endModal();
                });
            myForm.showModal();
        }

        cEntityForm::cEntityForm(
            wex::gui &parent,
            const std::string &catName,
            const std::vector<std::string> &propName)
            : myPanel(wex::maker::make<wex::panel>(parent)),
              myPG(wex::maker::make<wex::propertyGrid>(myPanel)),
              crud(wex::maker::make<wex::layout>(myPanel)),
              bnCreate(wex::maker::make<wex::button>(crud)),
              bnRead(wex::maker::make<wex::button>(crud)),
              bnUpdate(wex::maker::make<wex::button>(crud)),
              bnDelete(wex::maker::make<wex::button>(crud)),
              myCatName(catName),
              myPropName(propName)
        {
            myPanel.move({0, 0, 600, 800});

            myPG.move({10, 100, 550, 400});
            for (auto &s : myPropName)
                myPG.string(s, "");

            crud.move({20, 650, 500, 50});
            crud.grid(4);
            bnCreate.size(100, 50);
            bnCreate.text("Create");
            bnCreate.events().click(
                [this]
                { create(); });

            bnRead.size(100, 50);
            bnRead.text("Read");
            bnRead.events().click(
                [this]
                { read(); });

            bnUpdate.size(100, 50);
            bnUpdate.text("Update");
            bnUpdate.events().click(
                [this]
                { update(); });

            bnDelete.size(100, 50);
            bnDelete.text("Delete");
            bnDelete.events().click(
                [this]
                { remove(); });
        }
        void cEntityForm::show(const std::string &id, bool f)
        {
            myPanel.show(f);
            if (f)
            {
                if (id != "-1")
                {
                    raven::edb::cEntity p(myCatName, {});
                    p.read(id);
                    auto nv = p.nameValue();
                    pgset(p.nameValue());
                }
                // remember which entity is being edited
                myID = id;
            }
        }
        void cEntityForm::pgset(const std::vector<std::string> &vv)
        {
            for (int k = 0; k < myPropName.size(); k++)
                myPG.find(myPropName[k])->value(vv[2 * k + 1]);
        }
        std::vector<std::string> cEntityForm::pgget()
        {
            std::vector<std::string> ret;
            for ( auto& prop : myPropName )
            {
                ret.push_back(
                    myPG.find(prop)->value());
            }
            return ret;
        }
        void cEntityForm::create()
        {
            cEntity p(
                myCatName,
                pgget());
            p.add();
        }
        void cEntityForm::read()
        {
            cEntityList L(
                myCatName,
                myCatName);
            show(L.mySelected);
        }
        void cEntityForm::update()
        {
            if (myID == "-1")
            {
                create();
            }
            else
            {
                cEntity p(
                    myCatName,
                    pgget());
                p.id(myID);
                p.update();
            }
        }
        void cEntityForm::remove()
        {
            if (myID != "-1")
            {
                cEntity p(myCatName, {});
                p.id(myID);
                p.remove();
            }
            read();
        }
    }
}