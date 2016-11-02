class Controller {
  constructor(react, componentModel) {
    this.componentModel = cloneObject(componentModel);
    this.onChange = react.props.onChange;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      $.extend(true, this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  componentDidUpdate() {
    this.view.componentDidUpdate();
  }

  onSelectionChange(newValue) {
    this.onChange(newValue);
  }
}

export default Controller;
