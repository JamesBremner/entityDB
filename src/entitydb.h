#include <map>
#include "raven_sqlite.h"
#include "wex.h"
#include "propertygrid.h"

namespace raven
{
    namespace edb
    {

        /** Open database
         * @param[in] fname database filename
         * @return true if succesful
         * 
         * Applications must call this before doing any database operations
         */
        bool open(const std::string &fname);

        /** create category
         * @param[in] name
         * @param[in] van vector of attribute names
         * 
         * If the category is already in the database
         * the database specification is used.  Do not use to change the specification!
         * 
         * If a vategory of this name is not in the database
         * one will be added.
         * 
         */
        void createCategory(
            const std::string &name,
            const std::vector<std::string> &van);

        /**  Base class for all entities
         * 
         * Entities have attributes, which are name value pairs
         * e.g. attribute name "first name" might have value "James"
         * 
         * All entities that have the same attributes belong to a category
         * The entities of a category are instance of a specialized caless
         * based on this class.
         */
        class cEntity
        {
        public:
            /** CTOR
         * @param[in] categoryName
         * @param[in] vat vector of attribute values
         * 
         * If the category name is not present in the database
         * a new category will be created with the attributes specified
         * 
         */
            cEntity(
                const std::string &categoryName,
                const std::vector<std::string> &vat);

            /// read database attributes for this ID and category
            void read();

            /// read database attributes for this category and ID specified
            void read(const std::string &id)
            {
                myID = id;
                read();
            }

            /** add new entity to database
             * 
             * An ID number, unique for this entity in its category, will be assigned.
             */
            void add();

            /** update attribute values in this class instance
             * @param[in] vat vector of attribute values
             */
            void update(const std::vector<std::string> &vat)
            {
                myAt = vat;
            }

            /** update database
             * The attribute values in the database for the entity with this ID will be set
             * to the attributes in this class 
             */
            void update();

            /// delete entity with this ID from database
            void remove();

            std::vector<std::string> nameValue() const;
            std::string text() const;
            std::string id() const { return myID; }
            void id(const std::string &id) { myID = id; }
            int category() const { return myCategory; }

        protected:
            int myCategory;
            std::string myID;
            std::vector<std::string> myAt; // attribute values
        };

        class cDB
        {
        public:
            enum class error
            {
                ok,
                noUID,
                noDB,
                unknown,
                missing,
                out,
            };
            cDB() {}

            bool open(const std::string &fname);

            bool isOpen()
            {
                return db.isOpen();
            }

            int createCategory(
                const std::string &name,
                const std::vector<std::string> &van);

            /** Add new entity to database
             * @param[in] category
             * @param[in] nv name-values
             * @return ID of new entity, -1 on error
             */
            std::string add(
                int category,
                const std::vector<std::string> &nv);

            bool update(
                int category,
                const std::string &id,
                const std::vector<std::string> &nv);

            void remove(
                int category,
                const std::string &id);

            std::vector<std::string> read(
                int category,
                const std::string &id);

            std::vector<std::vector<std::string>> list(
                int cat);
            std::string text(
                int category,
                const std::string &id);

            /** get category id from name
             * @param[in] name
             * @return category id, -1 if no such category
             */
            int category(const std::string &name) const;
            std::string category(int catID) const;
            int categoryAttCount(int catID) const;

            /** get attribute name
             * @param[in] catID
             * @param[in] att 0-based attribute index
             * @return attrinute name
             */
            std::string categoryAttName(int catID, int att) const;

        private:
            raven::sqlite::cDB db;

            struct sCategory
            {
                std::string name;                 // name of category
                std::vector<std::string> attname; // attribute names
            };
            std::map<int, sCategory> myMapCategory;

            /** read category attribute names from database
             * param[in] catID
             * param[in] catName
             * 
             * The attribute names are stored, mapped to the category ID
             */
            void categoryRead(
                int catID,
                const std::string &catName);
        };

        /** Display all entities of a category in database
        *
        * Each row displays some attribute values for an entity.
        * The row title shows the entity's ID.  If clicked the display will close
        * and the mySelected will be the rowid clicked
        */
        class cEntityList
        {
        public:
            wex::gui &myForm;
            wex::propertyGrid &myPG;
            std::string mySelected;
            cEntityList(
                const std::string &title,
                const std::string &catName);
        };

        /// Edit entity details
        class cEntityForm
        {
        public:
            /** CTOR
         * @param[in] parent gui element
         * @param[in] catName category name
         * @param[in] propName vector of property names
         */
            cEntityForm(
                wex::gui &parent,
                const std::string &catName,
                const std::vector<std::string> &propName);
            void show(
                const std::string &id = "-1",
                bool f = true);
            void link(
                const std::string &propName,
                const std::string &linkName);

        private:
            wex::panel &myPanel;
            wex::propertyGrid &myPG;
            wex::layout &crud;
            wex::button &bnCreate;
            wex::button &bnRead;
            wex::button &bnUpdate;
            wex::button &bnDelete;

            std::vector<std::string> myPropName;
            std::string myCatName;
            std::string myID;
            std::map<std::string,std::string> myLinkMap;

            void pgset(const std::vector<std::string> &vv);
            std::vector<std::string> pgget();

            void nameClick(
                const std::string &nameClicked);

            void create();
            void read();
            void update();
            void remove();
        };
    }
}
