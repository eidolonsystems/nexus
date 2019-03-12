import { css, StyleSheet } from 'aphrodite/no-important';
import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, HLine } from '../../..';

/** The modes that the PhotoField can be displayed at. */
export enum PhotoFieldDisplayMode {

  /** Only the photo is visible. */
  DISPLAY,

  /** The uploader is visible. */
  UPLOADING
}

interface Properties {

  /** Determines the size to render the component at. */
  displaySize: DisplaySize;

  /** Determines if the ChangePictureModal is visible or not. */
  displayMode: PhotoFieldDisplayMode;

  /** The URL the image is located at. */
  imageSource?: string;

  /** Determines if the image can be changed or not. */
  readonly?: boolean;

  /** A value that determines how zoomed in the image will be.
   * It is a normalized scalar value.
   */
  scaling: number;

  /** The image displayed in the modal. A temporary image until the submit
   * button is clicked.
   */
  newImageSource: string;

  /** A value that determines how zoomed in the new image will be.
   * It is a normalized scalar value.
   */
  newScaling: number;

  /** Callback to hide or show the uploader. */
  onToggleUploader?: () => void;

  /** Callback to store the file and the scaling for the file. */
  onSubmit?: (newFileLocation: string, scaling: number) => void;

  /** Called to update the photo in the modal. */
  onNewPhotoChange?: (photo: string) => void;

  /** Called to update the scaling of the photo in the modal. */
  onNewScalingChange?: (scale: number) => void;
}

/** Displays an account's profile image. */
export class PhotoField extends React.Component<Properties, {}> {
  public static readonly defaultProps = {
    readonly: false,
    onToggleUploader: () => {},
    onSubmit: () => {},
    onNewPhotoChange: () => {},
    onNewScalingChange: () => {}
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return PhotoField.STYLE.boxSmall;
        case DisplaySize.MEDIUM:
          return PhotoField.STYLE.boxMedium;
        case DisplaySize.LARGE:
          return PhotoField.STYLE.boxLarge;
      }
    })();
    const cameraIconStyle = (() => {
      if(this.props.readonly) {
        return PhotoField.STYLE.hidden;
      } else {
        return PhotoField.STYLE.cameraIcon;
      }
    })();
    const cameraIconWrapper = (() => {
      if(this.props.readonly) {
        return PhotoField.STYLE.hidden;
      } else {
        return PhotoField.STYLE.cameraIconWrapper;
      }
    })();
    const imageSrc = (() => {
      if(this.props.imageSource) {
        return this.props.imageSource;
      } else {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      }
    })();
    const imageStyle = (() => {
      if(this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.imageSmall;
        } else {
          return PhotoField.STYLE.image;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.placeholderSmall;
        } else {
          return PhotoField.STYLE.placeholder;
        }
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${this.props.scaling})`
        });
      } else {
        return ({transform: 'scale(1)'});
      }
    })();
    return (
      <div style={PhotoField.STYLE.wrapper}>
        <div style={boxStyle}>
          <img src={imageSrc}
            style={{...imageStyle, ...imageScaling}}/>
          <div style={cameraIconWrapper}
              onClick={this.props.onToggleUploader}>
            <img src='resources/account_page/profile_page/camera.svg'
              style={cameraIconStyle}/>
          </div>
        </div>
        <Transition
            in={this.props.displayMode === PhotoFieldDisplayMode.UPLOADING}
            timeout={PhotoField.TIMEOUT}>
          {(state) => (
            <div style={{ ...PhotoField.STYLE.animationBase,
                ...(PhotoField.ANIMATION_STYLE as any)[state]}}>
              <ChangePictureModal displaySize={this.props.displaySize}
                imageSource={this.props.newImageSource}
                scaling={this.props.newScaling}
                onCloseModal={this.props.onToggleUploader}
                onSubmitImage={this.props.onSubmit}
                onPhotoChange={this.props.onNewPhotoChange}
                onScalingChange={this.props.onNewScalingChange}/>
            </div>)}
        </Transition>
      </div>);
  }

  private static ANIMATION_STYLE = {
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    },
    exited: {
      display: 'none' as 'none'
    }
  };
  private static readonly STYLE = {
    wrapper: {
      maxHeight: '288px',
      maxWidth: '424px'
    },
    animationBase: {
      opacity: 0,
      transition: 'opacity 200ms ease'
    },
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#F8F8F8',
      width: '100%',
      paddingTop: '68%',
      maxHeight: '288px',
      maxWidth: '424px',
      position: 'relative' as 'relative',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      overflow: 'hidden' as 'hidden'
    },
    boxMedium: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '190px',
      width: '280px',
      position: 'relative' as 'relative',
      overflow: 'hidden' as 'hidden'
    },
    boxLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '258px',
      width: '380px',
      position: 'relative' as 'relative',
      overflow: 'hidden' as 'hidden'
    },
    placeholder: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px'
    },
    placeholderSmall: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    },
    image: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageSmall: {
      position: 'absolute' as 'absolute',
      objectFit: 'cover' as 'cover',
      top: '0%',
      left: '0%',
      height: '100%',
      width: '100%'
    },
    cameraIcon: {
      height: '20px',
      width: '20px'
    },
    cameraIconWrapper: {
      height: '24px',
      width: '24px',
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      position: 'absolute' as 'absolute',
      top: 'calc(0% + 10px)',
      left: 'calc(100% - 10px - 24px)',
      cursor: 'pointer' as 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
  private static readonly TIMEOUT = 200;
}

interface ModalProperties {

  /** The image to be displayed. */
  imageSource?: string;

  /** A value that determines how zoomed in the image will be.
   * It is a normalized scalar value.
   */
  scaling: number;

  /** Determines the size at which to display the modal at. */
  displaySize: DisplaySize;

  /** Closes the modal. */
  onCloseModal?: () => void;

  /** Determines what happens when the file is submitted. */
  onSubmitImage?: (newFileLocation: string, scaling: number) => void;

  /** Called when the preview photo changes. */
  onPhotoChange?: (photo: string) => void;

  /** Called to change the slider when the slider moves. */
  onScalingChange?: (scale: number) => void;
}

/** Displays a modal that allows the user to change their picture. */
export class ChangePictureModal extends React.Component<ModalProperties> {
  public static readonly defaultProps = {
    onCloseModal: () => {},
    onSubmitImage: () => {},
    onPhotoChange: () => {},
    onScalingChange: () => {}
  };

  constructor(properties: ModalProperties) {
    super(properties);
    this.onSliderMovement = this.onSliderMovement.bind(this);
    this.onGetImageFile = this.onGetImageFile.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
    this.onClose = this.onClose.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.boxSmall;
      } else {
        return ChangePictureModal.STYLE.boxLarge;
      }
    })();
    const boxShadowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.boxShadowSmall;
      } else {
        return ChangePictureModal.STYLE.boxShadowLarge;
      }
    })();
    const buttonBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.buttonBoxSmall;
      } else {
        return ChangePictureModal.STYLE.buttonBoxLarge;
      }
    })();
    const imageSrc = (() => {
      if(this.props.imageSource) {
        return this.props.imageSource;
      } else {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      }
    })();
    const imageStyle = (() => {
      if(this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return ChangePictureModal.STYLE.imageSmall;
        } else {
          return ChangePictureModal.STYLE.imageLarge;
        }
      } else {
          return ChangePictureModal.STYLE.placeholderImage;
      }
    })();
    const imageBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.imageBoxSmall;
      } else {
        return ChangePictureModal.STYLE.imageBoxLarge;
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${this.props.scaling})`
        });
      } else {
        return { transform: 'scale(1)' };
      }
    })();
    return (
      <div>
        <div style={ChangePictureModal.STYLE.transparentBackground}/>
        <div style={boxShadowStyle}/>
        <HBoxLayout style={boxStyle}>
          <Padding size={ChangePictureModal.PADDING}/>
          <VBoxLayout>
            <Padding size={ChangePictureModal.PADDING}/>
            <div style={ChangePictureModal.STYLE.header}>
              {ChangePictureModal.HEADER_TEXT}
              <img src='resources/close.svg' tabIndex={0}
                style={ChangePictureModal.STYLE.closeIcon}
                onClick={this.onClose}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={imageBoxStyle}>
              <img src={imageSrc}
                style={{ ...imageStyle, ...imageScaling }}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <Slider onChange={this.onSliderMovement}
              scale={this.props.scaling}
              readonly={!this.props.imageSource}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <HLine color='#E6E6E6' height={1}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={buttonBox}>
              <input type='file' id='imageInput' accept='image/*'
                style={ChangePictureModal.STYLE.hiddenInput}
                tabIndex={0}
                onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                  this.onGetImageFile(event.target.files);}}/>
              <label htmlFor='imageInput' tabIndex={0}
                className={css(ChangePictureModal.DYNAMIC_STYLE.button)}>
                {ChangePictureModal.BROWSE_BUTTON_TEXT}
              </label>
              <div className={css(ChangePictureModal.DYNAMIC_STYLE.button)}
                onClick={this.onSubmit} tabIndex={0}>
                {ChangePictureModal.SUBMIT_BUTTON_TEXT}
              </div>
            </div>
            <Padding size={ChangePictureModal.PADDING}/>
          </VBoxLayout>
          <Padding size={ChangePictureModal.PADDING}/>
        </HBoxLayout>
      </div>);
  }

  private onSliderMovement(value: number) {
    this.props.onScalingChange(value);
  }

  private onGetImageFile(selectorFiles: FileList) {
    const file = selectorFiles.item(0);
    const someURL = URL.createObjectURL(file);
    this.props.onPhotoChange(someURL);
    this.setState({});
  }

  private onClose() {
    this.props.onCloseModal();
    this.setState({ scaling: 1 });
  }

  private onSubmit() {
    if(this.props.imageSource) {
      this.props.onSubmitImage(this.props.imageSource,
        this.props.scaling);
    }
    this.props.onCloseModal();
  }

  private static readonly STYLE = {
    transparentBackground: {
      boxSizing: 'border-box' as 'border-box',
      top: '0px',
      left: '0px',
      position: 'fixed' as 'fixed',
      width: '100%',
      height: '100%',
      backgroundColor: '#FFFFFF',
      opacity: 0.9
    },
    boxShadowSmall:{
      opacity: 0.4,
      display: 'block',
      boxShadow: '0px 0px 6px #000000',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '284px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxShadowLarge:{
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    boxSmall: {
      display: 'block',
      position: 'absolute' as 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '284px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxLarge: {
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    header: {
      display: 'flex' as 'flex',
      justifyContent: 'space-between' as 'space-between',
      font: '400 16px Roboto'
    },
    closeIcon: {
      width: '20px',
      height: '20px',
      cursor: 'pointer' as 'pointer'
    },
    buttonBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      flexWrap: 'wrap' as 'wrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      height: '86px'
    },
    buttonBoxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center'
    },
    placeholderImage: {
      position: 'relative' as 'relative',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    },
    imageSmall: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageLarge: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '166px',
      width: '248px',
      overflow: 'hidden' as 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    },
    imageBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      height: '216px',
      width: '324px',
      overflow: 'hidden' as 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    },
    hiddenInput: {
      width: '0.1px',
      height: '0.1px',
      opacity: 0,
      overflow: 'hidden' as 'hidden',
      position: 'absolute' as 'absolute'
    }
  };
  private static readonly DYNAMIC_STYLE = StyleSheet.create({
    button: {
      boxSizing: 'border-box' as 'border-box',
      cursor: 'pointer' as 'pointer',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      minWidth: '153px',
      maxWidth: '248px',
      height: '34px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      border: '1px solid #684BC7',
      borderRadius: '1px',
      outline: '0px',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      }
    }
  });
  private static readonly HEADER_TEXT = 'Change Picture';
  private static readonly BROWSE_BUTTON_TEXT = 'BROWSE';
  private static readonly SUBMIT_BUTTON_TEXT = 'SUBMIT';
  private static readonly PADDING = '18px';
  private static readonly PADDING_BETWEEN_ELEMENTS = '30px';
}

interface SliderProperties {

  /** Callback that updates the value */
  onChange?: (value: number) => void;

  /** Determines if the slider can be moved. */
  readonly?: boolean;

  /** The current slider value. */
  scale?: number;
}

/** Displays a slider that changes a value. */
export class Slider extends React.Component<SliderProperties, {}> {
  public static readonly defaultProps = {
    onChange: () => {},
    scale: 0,
    readonly: false
  };

  constructor(properties: SliderProperties) {
    super(properties);
    this.onValueChange = this.onValueChange.bind(this);
  }

  public render(): JSX.Element {
    return (<input type='range'
      min={Slider.MIN_RANGE_VALUE}
      max={Slider.MAX_RANGE_VALUE}
      value={Slider.convertFromDecimal(this.props.scale)}
      disabled={this.props.readonly}
      onChange={this.onValueChange}
      className={css(Slider.SLIDER_STYLE.slider)}/>);
  }

  private onValueChange(event: any) {
    const num = event.target.value;
    const diff = Math.abs(this.props.scale - num);
    if(this.props.scale < num) {
      this.props.onChange(Slider.convertToDecimal(
          this.props.scale + diff));
    } else {
      this.props.onChange(Slider.convertToDecimal(
          this.props.scale - diff));
    }
  }

  private static convertToDecimal(value: number) {
    return (100 + value) / 100;
  }

  private static convertFromDecimal(value: number) {
    return (value * 100) - 100;
  }

  public static readonly SLIDER_STYLE = StyleSheet.create({
    slider: {
      width: '100%',
      height: '20px',
      margin: '0px',
      outline: '0px',
      ':disabled' : {
        backgroundColor: '#FFFFFF'
      },
      '::-webkit-slider-thumb': {
        '-webkit-appearance': 'none',
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px',
        boxShadow: 'none',
        marginTop: '-8px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px',
        marginTop: '0px'
      },
      '::-webkit-slider-runnable-track': {
        '-webkit-appearance': 'none',
        boxShadow: 'none' as 'none',
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '::-moz-range-track': {
        backgroundColor: '#E6E6E6',
        height: '4px',
        border: '0px'
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '-webkit-appearance': 'none',
      '::-moz-focus-outer': {
        border: '0px'
      }
    }
  });
  private static readonly MIN_RANGE_VALUE = 0;
  private static readonly MAX_RANGE_VALUE = 200;
}
