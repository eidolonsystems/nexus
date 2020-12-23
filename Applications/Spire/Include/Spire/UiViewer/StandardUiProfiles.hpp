#ifndef SPIRE_STANDARD_UI_PROFILES_HPP
#define SPIRE_STANDARD_UI_PROFILES_HPP
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Returns a UiProfile for the CheckBox.
  UiProfile make_check_box_profile();

  //! Returns a UiProfile for the ColorSelectorButton.
  UiProfile make_color_selector_button_profile();

  //! Returns a UiProfile for the FlatButton.
  UiProfile make_flat_button_profile();
}

#endif
