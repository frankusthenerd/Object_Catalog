// =============================================================================
// Object Catalog Creator (Implementation)
// Programmed by Francois Lamini
// =============================================================================

#include "Object_Catalog.h"

Nerd::cPage_Manager* page_manager;

bool Layout_Process();
bool Process_Keys();

// **************************************************************************
// Program Entry Point
// **************************************************************************

int main(int argc, char** argv) {
  try {
    Nerd::cConfig config("Config");
    int width = config.Get_Property("width");
    int height = config.Get_Property("height");
    Nerd::cAllegro_IO allegro("Object Catalog", width, height, 1, "Game");
    allegro.Load_Resources_From_Files();
    page_manager = new Nerd::cPage_Manager();
    page_manager->Add_Page(new Nerd::cObject_Catalog(&allegro), "Layout");
    allegro.Process_Messages(Layout_Process, Process_Keys);
  }
  catch (Nerd::cError error) {
    error.Print();
  }
  if (page_manager) {
    delete page_manager;
  }
  std::cout << "Done." << std::endl;
  return 0;
}

// ****************************************************************************
// Layout Processor
// ****************************************************************************

/**
 * Called when command needs to be processed.
 * @return True if the app needs to exit, false otherwise.
 */
bool Layout_Process() {
  bool quit = false;
  try {
    page_manager->Render();
  }
  catch (Nerd::cError error) {
    error.Print();
    quit = true;
  }
  return quit;
}

/**
 * Called when keys are processed.
 * @return True if the app needs to exit, false otherwise.
 */
bool Process_Keys() {
  return false;
}

namespace Nerd {

  // **************************************************************************
  // Object Catalog Implementation
  // **************************************************************************

  /**
   * Creates a new object catalog.
   * @param io The I/O control.
   */
  cObject_Catalog::cObject_Catalog(cIO_Control* io) : cPage("Config", io) {

  }

  /**
   * Called when buttons are clicked.
   * @param entity The button entity.
   */
  void cObject_Catalog::On_Button_Click(tObject& entity) {
    tObject& catalog_name = this->components["catalog_name"];
    tObject& catalog_menu = this->components["catalog_menu"];
    tObject& object_name = this->components["object_name"];
    tObject& object_parent = this->components["object_parent"];
    tObject& catalog_objects = this->components["catalog_objects"];
    tObject& catalog_inspector = this->components["catalog_inspector"];
    if (entity["id"].string == "add_catalog") {
      if (catalog_name["text"].string.length() > 0) {
        this->Add_List_Item(catalog_menu, catalog_name["text"].string);
        this->Save_Catalog_Menu("Catalogs");
        catalog_name["text"].Set_String("");
      }
    }
    else if (entity["id"].string == "save_catalog") {
      if (catalog_name["text"].string.length() > 0) {
        this->Save_Catalog(catalog_name["text"].string);
      }
    }
    else if (entity["id"].string == "add_object") {
      if (object_name["text"].string.length() > 0) {
        this->Add_List_Item(catalog_objects, object_name["text"].string);
        std::string name = object_name["text"].string;
        Check_Condition(!this->catalog.Does_Key_Exist(name), "Object " + name + " already exists.");
        if (object_parent["text"].string.length() > 0) {
          std::string parent = object_parent["text"].string;
          if (this->catalog.Does_Key_Exist(parent)) {
            tObject object;
            object["parent"].Set_String(parent); // Add parent property.
            this->catalog[name] = object;
            int prop_count = this->catalog[parent].Count();
            for (int prop_index = 0; prop_index < prop_count; prop_index++) {
              std::string property = this->catalog[parent].keys[prop_index];
              if (property != "parent") {
                if (property.find("*") == 0) {
                  this->catalog[name][property].Set_String(this->catalog[parent][property].string);
                }
                else {
                  this->catalog[name]["*" + property] = this->catalog[parent][property]; // Mark properties from parent.
                }
              }
            }
          }
          else {
            this->catalog[name] = tObject();
          }
        }
        else {
          this->catalog[name] = tObject();
        }
      }
    }
    else if (entity["id"].string == "delete_object") {
      if (object_name["text"].string.length() > 0) {
        std::string name = object_name["text"].string;
        if (this->catalog.Does_Key_Exist(name)) {
          this->catalog.Remove(name);
          this->Remove_List_Item(catalog_objects, catalog_objects["sel-item"].number);
          this->Clear_Grid(catalog_inspector);
        }
      }
    }
    else if (entity["id"].string == "update_object") {
      if (object_name["text"].string.length() > 0) {
        std::string name = object_name["text"].string;
        if (!this->catalog.Does_Key_Exist(name)) {
          this->catalog[name] = tObject();
        }
        this->Load_Object_From_Grid(this->catalog[name]);
      }
    }
    else if (entity["id"].string == "rescan_parent") {
      if (object_name["text"].string.length() > 0) {
        std::string name = object_name["text"].string;
        this->Rescan_Parent_Object(this->catalog[name]);
        this->Save_Object_To_Grid(this->catalog[name]);
      }
    }
  }

  /**
   * Called when lists are clicked.
   * @param entity The list entity.
   * @param text The text of the item that was clicked.
   */
  void cObject_Catalog::On_List_Click(tObject& entity, std::string text) {
    tObject& catalog_name = this->components["catalog_name"];
    tObject& object_name = this->components["object_name"];
    tObject& object_parent = this->components["object_parent"];
    if (entity["id"].string == "catalog_menu") {
      catalog_name["text"].Set_String(text);
      this->Load_Catalog(text);
    }
    else if (entity["id"].string == "catalog_objects") {
      std::string name = text;
      if (this->catalog.Does_Key_Exist(name)) {
        this->Save_Object_To_Grid(this->catalog[name]);
        object_name["text"].Set_String(name);
        object_parent["text"].Set_String("");
      }
    }
  }

  /**
   * Called when the catalog is initialized.
   */
  void cObject_Catalog::On_Init() {
    this->Load_Catalog_Menu("Catalogs");
  }

  /**
   * Loads the catalog.
   * @param name The name of the catalog to load.
   */
  void cObject_Catalog::Load_Catalog(std::string name) {
    try {
      cFile file(name + ".txt");
      file.Read();
      this->Clear_Catalog();
      while (file.Has_More_Lines()) {
        tObject object;
        std::string name = file.Get_Line();
        file >>= object;
        this->catalog[name] = object;
        this->Add_List_Item(this->components["catalog_objects"], name);
      }
    }
    catch (cError error) {
      this->Clear_Catalog();
      error.Print();
    }
  }

  /**
   * Saves the catalog.
   * @param name The name of the catalog.
   * @throws An error if the catalog could not be saved.
   */
  void cObject_Catalog::Save_Catalog(std::string name) {
    cFile file(name + ".txt");
    int obj_count = this->catalog.Count();
    for (int obj_index = 0; obj_index < obj_count; obj_index++) {
      std::string obj_name = this->catalog.keys[obj_index];
      tObject& object = this->catalog.values[obj_index];
      file.Add(obj_name);
      file.Add(object);
    }
    file.Write();
  }

  /**
   * Loads the catalog menu.
   * @param name The name of the menu to load.
   */
  void cObject_Catalog::Load_Catalog_Menu(std::string name) {
    try {
      cFile file(name + ".txt");
      file.Read();
      this->Clear_List(this->components["catalog_menu"]);
      while (file.Has_More_Lines()) {
        std::string item = file.Get_Line();
        this->Add_List_Item(this->components["catalog_menu"], item);
      }
    }
    catch (cError error) {
      error.Print();
    }
  }

  /**
   * Saves the catalog menu.
   * @param name The name of the menu to save.
   */
  void cObject_Catalog::Save_Catalog_Menu(std::string name) {
    cFile file(name + ".txt");
    tObject& catalog_menu = this->components["catalog_menu"];
    int item_count = this->Get_List_Item_Count(catalog_menu);
    for (int item_index = 0; item_index < item_count; item_index++) {
      std::string item = this->Get_List_Item(catalog_menu, item_index);
      file.Add(item);
    }
    file.Write();
  }

  /**
   * Rescans the parent object and updates the parent properties.
   * @param object The subclass object.
   */
  void cObject_Catalog::Rescan_Parent_Object(tObject& object) {
    if (object.Does_Key_Exist("parent")) {
      std::string parent_name = object["parent"].string;
      if (this->catalog.Does_Key_Exist(parent_name)) {
        tObject& parent = this->catalog[parent_name];
        int prop_count = object.Count();
        for (int prop_index = 0; prop_index < prop_count; prop_index++) {
          std::string property = object.keys[prop_index];
          if (property.find("*") == 0) { // Starred property.
            std::string name = property.substr(1);
            if (!parent.Does_Key_Exist(name)) { // Not in base?
              object.Remove(property); // Remove property.
            }
          }
        }
        prop_count = parent.Count();
        for (int prop_index = 0; prop_index < prop_count; prop_index++) {
          std::string property = parent.keys[prop_index];
          if (!object.Does_Key_Exist("*" + property)) { // Property new to object?
            object["*" + property] = parent[property]; // Add in property.
          }
        }
      }
    }
  }

  /**
   * Destars a starred object.
   * @param object The object to unstar.
   */
  void cObject_Catalog::Destar_Object(tObject& object) {
    tObject new_object;
    int prop_count = object.Count();
    for (int prop_index = 0; prop_index < prop_count; prop_index++) {
      std::string property = object.keys[prop_index];
      if (property.find("*") == 0) {
        std::string name = property.substr(1);
        new_object[name] = object[property];
      }
      else {
        new_object[property] = object[property];
      }
    }
  }

  /**
   * Loads an object from the inspector.
   * @param object The object.
   */
  void cObject_Catalog::Load_Object_From_Grid(tObject& object) {
    object.Clear();
    tObject& catalog_inspector = this->components["catalog_inspector"];
    int row_count = catalog_inspector["rows"].number;
    for (int row_index = 0; row_index < row_count; row_index++) {
      std::string name = this->Get_Grid_Cell(catalog_inspector, 0, row_index);
      std::string value = this->Get_Grid_Cell(catalog_inspector, 1, row_index);
      if ((name.length() > 0) && (name != "free")) { // Ignore blank names.
        object[name] = value;
      }
    }
  }

  /**
   * Saves an object to the inspector.
   * @param object The object.
   * @throws An error if the object has too many properties.
   */
  void cObject_Catalog::Save_Object_To_Grid(tObject& object) {
    tObject& catalog_inspector = this->components["catalog_inspector"];
    int prop_count = object.Count();
    Check_Condition((prop_count <= catalog_inspector["rows"].number), "Too many properties in object.");
    this->Clear_Grid(catalog_inspector);
    for (int prop_index = 0; prop_index < prop_count; prop_index++) {
      std::string name = object.keys[prop_index];
      std::string value = object[name].string;
      this->Set_Grid_Cell(catalog_inspector, 0, prop_index, name);
      this->Set_Grid_Cell(catalog_inspector, 1, prop_index, value);
    }
  }

  /**
   * Clears out the loaded catalog.
   */
  void cObject_Catalog::Clear_Catalog() {
    this->Clear_List(this->components["catalog_objects"]);
    this->catalog.Clear();
    this->Clear_Grid(this->components["catalog_inspector"]);
    this->components["object_name"]["text"].Set_String("");
    this->components["object_parent"]["text"].Set_String("");
  }

}
