// =============================================================================
// Object Catalog Creator (Definitions)
// Programmed by Francois Lamini
// =============================================================================

#include "..\Nerd\Nerd.hpp"
#include "..\Nerd\Allegro.hpp"

namespace Nerd {

  class cObject_Catalog : public cPage {

    public:
      cHash<std::string, tObject> catalog;

      cObject_Catalog(cIO_Control* io);
      void On_Button_Click(tObject& entity);
      void On_List_Click(tObject& entity, std::string text);
      void On_Init();
      void Load_Catalog(std::string name);
      void Save_Catalog(std::string name);
      void Load_Catalog_Menu(std::string name);
      void Save_Catalog_Menu(std::string name);
      void Rescan_Parent_Object(tObject& object);
      void Destar_Object(tObject& object);
      void Load_Object_From_Grid(tObject& object);
      void Save_Object_To_Grid(tObject& object);
      void Clear_Catalog();

  };

}
