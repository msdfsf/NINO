// just to clean up main fail a bit, so all ui stuff, that is mainly global will be there
// P.S. The worst idea...

#pragma once

#include "Config.h"
#include "Resources.h"
#include "Render.h"
#include "Color.h"

#include "AudioDriver.h"
#include "AudioIOWASAPI.h"

#include "Control.h"
#include "Window.h"
#include "Panel.h"
#include "Button.h"
#include "Image.h"
#include "SelectMenu.h"
#include "DropDownSelectMenu.h"
#include "Checkbox.h"
#include "Input.h"
#include "Cursor.h"
#include "System.h"

#include "PluginContainer.h"

Window* window;
PluginContainer* pluginContainer;

Control* pluginSelect;
SelectMenu* pluginSelectMenu;

Control* presetSelect;
SelectMenu* presetSelectMenu;

Control* settingsMenu;

int ASIOSettingsHeight = 0;
int WASAPISettingsHeight = 0;
Control* stDriverSpecificSection;

Image* cpPluginsButtonIcon;
Image* cpPresetsButtonIcon;
Image* cpSettingsButtonIcon;

Control* basicAudioControlHeader;
Image* audioDriverOnOffSwitch;

Control* selectedControlPanelMenu = NULL;
Button* selectedControlPanelButton = NULL;
Image* selectedControlPanelImage = NULL;

Control* presetLabel;
Control* savePresetAsPopup;
Input* savePresetAsPopupNameInput;
Input* savePresetAsPopupFileInput;

// settings
DropDownSelectMenu* stDeviceSelect;

void toggleASIOSettings(int state);
void toggleWASAPISettings(int state);
void toggleFileSettings(int state);

void (*toggleLastSettings) (int) = &toggleASIOSettings;

// ASIO
Control* stASIOConfigLabel;
Button* stASIOConfig;

// WASAPI
Input* stWASAPIBufferDuration;

Control* stWASAPIShareModeLabel;
Checkbox* stWASAPIShareMode;

Checkbox* stInLeftChannel;
Checkbox* stOutLeftChannel;
Checkbox* stInRightChannel;
Checkbox* stOutRightChannel;

// etc

Preset* selectedPreset = NULL;

void selectControlMenuButton(Button* button, Control* menu, Image* image);

void presetsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void pluginsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void settingsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void pluginMenuDragStart(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void pluginMenuDragEnd(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void pluginMenuDrag(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void pluginMenuDrop(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void pluginContainerDrop(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void changeAudioDriver(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void changeAudioDevice(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void stUIResizeTypeChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void stInLeftChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void stInRightChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void stOutLeftChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void stOutRightChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void openASIOConfig(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void audioDriverOnOffSwitchClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void pluginSelectDblClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void presetSelectDblClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void setResizeType(ResizeType::ResizeType type);
void setPresets(std::vector<Preset*> presets);
void openPreset(Preset* preset);
void clearRack();
void closePreset();
void savePreset(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void savePresetAs(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void showSavePresetAsPopup(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);
void hideSavePresetAsPopup(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

void shareModeWASAPIChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB);

Control* buildSettingsLabel(Control* parent, int y, int height, int leftPadding, char* text);

// init basic root ui control
int initWindow() {

    window = (Window*)Control::window;

    window->x = 0;
    window->y = 0;
    window->width = Config::renderWidth;
    window->height = Config::renderHeight;

    window->borderColor = Color::BLUE;

    return 1;

}

int initBasicControls() {

    Panel* controlPanel = new Panel();
    Panel* rackPanel = new Panel();

    window->addControl(controlPanel);
    window->addControl(rackPanel);

    controlPanel->setWidth(0.5);
    controlPanel->setHeight(1.0);
    controlPanel->borderRightWidth = 1;
    controlPanel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    controlPanel->borderColor = MAIN_FRONT_COLOR;

    rackPanel->setX(0.5);
    rackPanel->setWidth(0.5);
    rackPanel->setHeight(1.0);
    rackPanel->borderLeftWidth = 1;
    rackPanel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    rackPanel->borderColor = MAIN_FRONT_COLOR;

    // presets button
    Button* cpPresetsButton = new Button();

    controlPanel->addControl(cpPresetsButton);

    cpPresetsButton->setX(1.0 / 3);
    cpPresetsButton->setWidth(1.0 / 3);
    cpPresetsButton->setHeight(32);
    cpPresetsButton->text = (char*)"Presets";
    cpPresetsButton->textLength = 7;
    cpPresetsButton->fontSize = 10;
    cpPresetsButton->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    cpPresetsButton->borderColor = MAIN_FRONT_COLOR;
    cpPresetsButton->selectBackColor = MAIN_FRONT_COLOR;
    cpPresetsButton->selectFrontColor = MAIN_BACK_COLOR;
    cpPresetsButton->setBorderWidth(4);
    cpPresetsButton->setPadding(4);
    cpPresetsButton->textAlignment = StringAlignment::RIGHT;

    cpPresetsButton->eMouseClick = &presetsButtonClick;

    // presets button icon
    cpPresetsButtonIcon = new Image();

    cpPresetsButton->addControl(cpPresetsButtonIcon);

    cpPresetsButtonIcon->setX(0.0);
    cpPresetsButtonIcon->setY(0.0);
    cpPresetsButtonIcon->setHeight(1.0);
    cpPresetsButtonIcon->width = cpPresetsButtonIcon->height;
    cpPresetsButtonIcon->drawType = ImageDrawType::ONE_COLOR;
    cpPresetsButtonIcon->color = MAIN_FRONT_COLOR;
    cpPresetsButtonIcon->backgroundColor = MAIN_BACK_COLOR;
    cpPresetsButtonIcon->selected = 0;
    cpPresetsButtonIcon->setImage((Render::Bitmap*) Resources::presetsIconInv);

    // plugins button
    Button* cpPluginsButton = new Button();

    controlPanel->addControl(cpPluginsButton);

    cpPluginsButton->setX(0.0);
    cpPluginsButton->setWidth(1.0 / 3);
    cpPluginsButton->setHeight(32);
    cpPluginsButton->text = (char*)"Plugins";
    cpPluginsButton->textLength = 7;
    cpPluginsButton->fontSize = 10;
    cpPluginsButton->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    cpPluginsButton->borderColor = MAIN_FRONT_COLOR;
    cpPluginsButton->selectBackColor = MAIN_FRONT_COLOR;
    cpPluginsButton->selectFrontColor = MAIN_BACK_COLOR;
    cpPluginsButton->setBorderWidth(4);
    cpPluginsButton->setPadding(4);
    cpPluginsButton->textAlignment = StringAlignment::RIGHT;

    cpPluginsButton->select();

    cpPluginsButton->eMouseClick = &pluginsButtonClick;

    // plugins icon
    cpPluginsButtonIcon = new Image();

    cpPluginsButton->addControl(cpPluginsButtonIcon);

    cpPluginsButtonIcon->setX(0.0);
    cpPluginsButtonIcon->setY(0.0);
    cpPluginsButtonIcon->setHeight(1.0);
    cpPluginsButtonIcon->width = cpPluginsButtonIcon->height;
    cpPluginsButtonIcon->drawType = ImageDrawType::ONE_COLOR;
    cpPluginsButtonIcon->color = MAIN_FRONT_COLOR;
    cpPluginsButtonIcon->backgroundColor = MAIN_BACK_COLOR;
    cpPluginsButtonIcon->selected = 1;
    cpPluginsButtonIcon->setImage((Render::Bitmap*) Resources::pluginsIconInv);

    // settings button
    Button* cpSettingsButton = new Button();

    controlPanel->addControl(cpSettingsButton);

    cpSettingsButton->setX(2.0 / 3);
    cpSettingsButton->setWidth(1.0 / 3);
    cpSettingsButton->setHeight(32);
    cpSettingsButton->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    cpSettingsButton->selectBackColor = MAIN_FRONT_COLOR;
    cpSettingsButton->selectFrontColor = MAIN_BACK_COLOR;
    cpSettingsButton->text = (char*)"Settings";
    cpSettingsButton->textLength = 8;
    cpSettingsButton->fontSize = 10;
    cpSettingsButton->borderColor = MAIN_FRONT_COLOR;
    cpSettingsButton->setBorderWidth(4);
    cpSettingsButton->setPadding(4);
    cpSettingsButton->textAlignment = StringAlignment::RIGHT;

    cpSettingsButton->eMouseClick = &settingsButtonClick;

    // settings button icon
    cpSettingsButtonIcon = new Image();

    cpSettingsButton->addControl(cpSettingsButtonIcon);

    cpSettingsButtonIcon->setX(0.0);
    cpSettingsButtonIcon->setY(0.0);
    cpSettingsButtonIcon->setHeight(1.0);
    cpSettingsButtonIcon->width = cpSettingsButtonIcon->height;
    cpSettingsButtonIcon->drawType = ImageDrawType::ONE_COLOR;
    cpSettingsButtonIcon->color = MAIN_FRONT_COLOR;
    cpSettingsButtonIcon->backgroundColor = MAIN_BACK_COLOR;
    cpSettingsButtonIcon->selected = 0;
    cpSettingsButtonIcon->setImage((Render::Bitmap*) Resources::settingsIconInv);

    // presets select menu
    presetSelect = new Control();
    controlPanel->addControl(presetSelect);

    presetSelect->setX(0.0);
    presetSelect->setY(32);
    presetSelect->setWidth(1.0);
    presetSelect->setHeight(1.0);
    presetSelect->height = presetSelect->height - presetSelect->y;
    presetSelect->setPadding(4);
    presetSelect->setBorderWidth(4);
    presetSelect->innerBorderBevelWidth = 2;
    presetSelect->borderColor = MAIN_FRONT_COLOR;
    presetSelect->backgroundColor = MAIN_BACK_COLOR;

    presetSelectMenu = new SelectMenu();
    presetSelect->addControl(presetSelectMenu);

    presetSelectMenu->setX(0.0);
    presetSelectMenu->setY(0.0);
    presetSelectMenu->setBorderWidth(4);
    presetSelectMenu->outerBorderBevelWidth = 2;
    presetSelectMenu->setPadding(2);
    presetSelectMenu->borderColor = MAIN_FRONT_COLOR;
    presetSelectMenu->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    presetSelectMenu->hoverFrontColor = MAIN_BACK_COLOR;
    presetSelectMenu->hoverBackColor = MAIN_HIGHLIGHT_COLOR;
    presetSelectMenu->setWidth(1.0);
    presetSelectMenu->setHeight(1.0);
    presetSelectMenu->fontSize = 15;
    presetSelectMenu->textAlignment = StringAlignment::LEFT;
    
    presetSelectMenu->eMouseDblClick = &presetSelectDblClick;

    presetSelect->visible = 0;

    // pluginSelectMenu = new SelectMenu((char**)testStr, sizeof(testStr) / sizeof(int*));
    pluginSelect = new Control();
    controlPanel->addControl(pluginSelect);
    
    pluginSelect->setX(0.0);
    pluginSelect->setY(32);
    pluginSelect->setWidth(1.0);
    pluginSelect->setHeight(1.0);
    pluginSelect->height = pluginSelect->height - pluginSelect->y;
    pluginSelect->setPadding(4);
    pluginSelect->setBorderWidth(4);
    pluginSelect->innerBorderBevelWidth = 2;
    pluginSelect->borderColor = MAIN_FRONT_COLOR;
    pluginSelect->backgroundColor = MAIN_BACK_COLOR;

    pluginSelectMenu = new SelectMenu();
    pluginSelect->addControl(pluginSelectMenu);

    pluginSelectMenu->draggable = 1;
    pluginSelectMenu->eDragStart = &pluginMenuDragStart;
    pluginSelectMenu->eDragEnd = &pluginMenuDragEnd;
    pluginSelectMenu->eDrag = &pluginMenuDrag;

    pluginSelectMenu->setX(0.0);
    pluginSelectMenu->setY(0.0);
    pluginSelectMenu->setBorderWidth(4);
    pluginSelectMenu->outerBorderBevelWidth = 2;
    pluginSelectMenu->borderColor = MAIN_FRONT_COLOR;
    pluginSelectMenu->setPadding(2);
    pluginSelectMenu->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR); \
    pluginSelectMenu->hoverFrontColor = MAIN_BACK_COLOR;
    pluginSelectMenu->hoverBackColor = MAIN_HIGHLIGHT_COLOR;
    pluginSelectMenu->setWidth(1.0);
    pluginSelectMenu->setHeight(1.0);
    //pluginSelectMenu->height = pluginSelectMenu->height - 32;
    pluginSelectMenu->fontSize = 15;
    pluginSelectMenu->textAlignment = StringAlignment::LEFT;
    pluginSelectMenu->setItemHeight();

    pluginSelectMenu->eMouseDblClick = &pluginSelectDblClick;

    // settings menu
    settingsMenu = new Control();
    controlPanel->addControl(settingsMenu);

    settingsMenu->setY(32);
    settingsMenu->setBorderWidth(4);
    settingsMenu->innerBorderBevelWidth = 2;
    settingsMenu->setPadding(4);
    settingsMenu->borderColor = MAIN_FRONT_COLOR;
    settingsMenu->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    settingsMenu->setWidth(1.0);
    settingsMenu->setHeight(1.0);
    settingsMenu->height = settingsMenu->height - 32;
    settingsMenu->fontSize = 15;
    settingsMenu->textAlignment = StringAlignment::CENTER;
    settingsMenu->visible = 0;

    const int settingsItemBorderWidth = 4;
    const int settingsRowSpace = 8;
    const int settingsItemHeight = 50;
    const int settingsItemFontSize = 12;
    const int stLeftPadding = 10;

    Control* stMainSection = new Control();
    settingsMenu->addControl(stMainSection);

    stMainSection->setX(0.0);
    stMainSection->setY(0.0);
    stMainSection->y += 4;
    stMainSection->setPadding(settingsRowSpace);
    stMainSection->setHeight(2 * settingsItemHeight + 2 * settingsRowSpace + 2 * settingsItemBorderWidth);
    stMainSection->setWidth(1.0);
    stMainSection->setBorderWidth(settingsItemBorderWidth);
    stMainSection->borderColor = MAIN_FRONT_COLOR;
    stMainSection->outerBorderBevelWidth = 2;
    stMainSection->setBorderTitle((char*) "Main", 4);
    stMainSection->borderTitlePadding = 10;
    stMainSection->borderTitleFontSize = 10;

    // driver slection
    //
    int y = 0;

    // label
    Control* stDriverSelectLabel = buildSettingsLabel(
        stMainSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*) "Driver"
    );
    stDriverSelectLabel->setY(0.0);

    // menu
    const char* stDriverOptions[] = {
        "ASIO",
        "WASAPI",
        "LEGACY",
        "FILE"
    };

    DropDownSelectMenu* stDriverSelect = new DropDownSelectMenu((char**)stDriverOptions, 4);
    stMainSection->addControl(stDriverSelect);

    stDriverSelect->setX(0.0);
    stDriverSelect->x += stDriverSelectLabel->width;
    stDriverSelect->setY(0.0);
    stDriverSelect->setWidth(0.5);
    stDriverSelect->setHeight(settingsItemHeight);
    stDriverSelect->paddingLeft = stLeftPadding;
    stDriverSelect->textAlignment = StringAlignment::LEFT;
    stDriverSelect->text = (char*)stDriverOptions[0];
    stDriverSelect->textLength = 4;
    stDriverSelect->fontSize = settingsItemFontSize;
    stDriverSelect->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);

    stDriverSelect->eChange = &changeAudioDriver;

    // device slection
    //
    y = stDriverSelect->y + stDriverSelect->height;

    // label
    Control* stDeviceSelectLabel = buildSettingsLabel(
        stMainSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*) "Device"
    );

    // menu
    const char* stDeviceOptions[] = {
        "Default"
    };

    stDeviceSelect = new DropDownSelectMenu((char**)stDeviceOptions, 1);
    stMainSection->addControl(stDeviceSelect);

    stDeviceSelect->setX(0.0);
    stDeviceSelect->x += stDeviceSelectLabel->width;
    stDeviceSelect->setY(y);
    stDeviceSelect->setWidth(0.5);
    stDeviceSelect->setHeight(settingsItemHeight);
    stDeviceSelect->paddingLeft = stLeftPadding;
    stDeviceSelect->textAlignment = StringAlignment::LEFT;
    stDeviceSelect->text = (char*)stDeviceOptions[0];
    stDeviceSelect->textLength = 7;
    stDeviceSelect->fontSize = settingsItemFontSize;
    stDeviceSelect->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);

    stDeviceSelect->eChange = &changeAudioDevice;


    //
    // UI settings
    //

    Control* stUISpecificSection = new Control();
    settingsMenu->addControl(stUISpecificSection);

    stUISpecificSection->setX(0.0);
    stUISpecificSection->setY(stMainSection->y + stMainSection->height + settingsRowSpace);
    stUISpecificSection->setHeight(settingsItemHeight + 2 * settingsItemBorderWidth + 2 * settingsRowSpace);
    stUISpecificSection->setWidth(1.0);
    stUISpecificSection->setBorderWidth(4);
    stUISpecificSection->borderColor = MAIN_FRONT_COLOR;
    stUISpecificSection->outerBorderBevelWidth = 2;
    stUISpecificSection->setPadding(settingsRowSpace);
    stUISpecificSection->setBorderTitle((char*)"UI", 2);
    stUISpecificSection->borderTitlePadding = 10;
    stUISpecificSection->borderTitleFontSize = 10;

    y = stUISpecificSection->getInBoxY();

    // resize type
    Control* stUIResizeTypeLabel = buildSettingsLabel(
        stUISpecificSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"Resize Type"
    );

    const char* stUIResizeTypeOptions[] = {
        "NONE",
        "SCALE"
    };

    DropDownSelectMenu* stUIResizeType = new DropDownSelectMenu((char**)stUIResizeTypeOptions, 2);
    stUISpecificSection->addControl(stUIResizeType);

    stUIResizeType->setX(0.0);
    stUIResizeType->x += stUIResizeTypeLabel->width;
    stUIResizeType->setY(y);
    stUIResizeType->setWidth(0.5);
    stUIResizeType->setHeight(settingsItemHeight);
    stUIResizeType->paddingLeft = stLeftPadding;
    stUIResizeType->setText((char*)stUIResizeTypeOptions[Config::windowResize], strlen(stUIResizeTypeOptions[Config::windowResize]));
    stUIResizeType->textAlignment = StringAlignment::LEFT;
    stUIResizeType->selectOption(Config::windowResize);
    //stWASAPIShareMode->text = (char*)"Open";
    //stWASAPIShareMode->textLength = 4;
    stUIResizeType->fontSize = settingsItemFontSize;
    stUIResizeType->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stUIResizeType->eChange = &stUIResizeTypeChange;



    //
    // Channels
    //

    Control* stChannelsSection = new Control();
    settingsMenu->addControl(stChannelsSection);

    stChannelsSection->setX(0.0);
    stChannelsSection->setY(stUISpecificSection->y + stUISpecificSection->height + settingsRowSpace);
    stChannelsSection->setHeight(4 * settingsItemHeight + 2 * settingsItemBorderWidth + 2 * settingsRowSpace);
    stChannelsSection->setWidth(1.0);
    stChannelsSection->setBorderWidth(settingsItemBorderWidth);
    stChannelsSection->borderColor = MAIN_FRONT_COLOR;
    stChannelsSection->outerBorderBevelWidth = 2;
    stChannelsSection->setPadding(settingsRowSpace);
    stChannelsSection->setBorderTitle((char*) "Channels", 8);
    stChannelsSection->borderTitlePadding = 10;
    stChannelsSection->borderTitleFontSize = 10;

    y = stChannelsSection->getInBoxY();

    // checkbox
    const int checkboxHeight = 1 * settingsItemHeight / 2;
    const int checkboxOffsetY = (settingsItemHeight - checkboxHeight) / 2;

    // input left channel
    // 

    // label
    Control* stInLeftChannelLabel = buildSettingsLabel(
        stChannelsSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"In L"
    );

    // checkbox
    stInLeftChannel = new Checkbox();
    stChannelsSection->addControl(stInLeftChannel);

    stInLeftChannel->setX(0.0);
    stInLeftChannel->x += stInLeftChannelLabel->width + stLeftPadding;
    stInLeftChannel->setY(y + checkboxOffsetY);
    stInLeftChannel->setWidth(0.5);
    stInLeftChannel->setHeight(checkboxHeight);
    stInLeftChannel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stInLeftChannel->eChange = &stInLeftChannelChange;
    stInLeftChannel->setCheckType(Checkbox::CheckType::SQUARE);

    // input right channel
    // 
    y = stInLeftChannelLabel->y + stInLeftChannelLabel->height;

    // label
    Control* stInRightChannelLabel = buildSettingsLabel(
        stChannelsSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"In R"
    );

    // checkbox
    stInRightChannel = new Checkbox();
    stChannelsSection->addControl(stInRightChannel);

    stInRightChannel->setX(0.0);
    stInRightChannel->x += stInRightChannelLabel->width + stLeftPadding;
    stInRightChannel->setY(y + checkboxOffsetY);
    stInRightChannel->setWidth(0.5);
    stInRightChannel->setHeight(checkboxHeight);
    stInRightChannel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stInRightChannel->eChange = &stInRightChannelChange;
    stInRightChannel->setCheckType(Checkbox::CheckType::SQUARE);

    // output left channel
    // 
    y = stInRightChannelLabel->y + stInRightChannelLabel->height;

    // label
    Control* stOutLeftChannelLabel = buildSettingsLabel(
        stChannelsSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"Out L"
    );

    // checkbox
    stOutLeftChannel = new Checkbox();
    stChannelsSection->addControl(stOutLeftChannel);

    stOutLeftChannel->setX(0.0);
    stOutLeftChannel->x += stOutLeftChannelLabel->width + stLeftPadding;
    stOutLeftChannel->setY(y + checkboxOffsetY);
    stOutLeftChannel->setWidth(0.5);
    stOutLeftChannel->setHeight(checkboxHeight);
    stOutLeftChannel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stOutLeftChannel->eChange = &stOutLeftChannelChange;
    stOutLeftChannel->setCheckType(Checkbox::CheckType::SQUARE);

    // output right channel
    // 
    y = stOutLeftChannelLabel->y + stOutLeftChannelLabel->height;

    // label
    Control* stOutRightChannelLabel = buildSettingsLabel(
        stChannelsSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"Out R"
    );

    // checkbox
    stOutRightChannel = new Checkbox();
    stChannelsSection->addControl(stOutRightChannel);

    stOutRightChannel->setX(0.0);
    stOutRightChannel->x += stOutRightChannelLabel->width + stLeftPadding;
    stOutRightChannel->setY(y + checkboxOffsetY);
    stOutRightChannel->setWidth(0.5);
    stOutRightChannel->setHeight(checkboxHeight);
    stOutRightChannel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stOutRightChannel->eChange = &stOutRightChannelChange;
    stOutRightChannel->setCheckType(Checkbox::CheckType::SQUARE);

    //
    // ASIO specific settings
    //

    stDriverSpecificSection = new Control();
    settingsMenu->addControl(stDriverSpecificSection);

    stDriverSpecificSection->setX(0.0);
    stDriverSpecificSection->setY(stChannelsSection->y + stChannelsSection->height + settingsRowSpace);
    stDriverSpecificSection->setHeight(settingsItemHeight + 2 * settingsItemBorderWidth + 2 * settingsRowSpace);
    stDriverSpecificSection->setWidth(1.0);
    stDriverSpecificSection->setBorderWidth(4);
    stDriverSpecificSection->borderColor = MAIN_FRONT_COLOR;
    stDriverSpecificSection->outerBorderBevelWidth = 2;
    stDriverSpecificSection->setPadding(settingsRowSpace);
    stDriverSpecificSection->setBorderTitle((char*)"Driver Specific", 15);
    stDriverSpecificSection->borderTitlePadding = 10;
    stDriverSpecificSection->borderTitleFontSize = 10;

    y = stDriverSpecificSection->getInBoxY();

    ASIOSettingsHeight = settingsItemHeight + 2 * settingsItemBorderWidth + 2 * settingsRowSpace;
    WASAPISettingsHeight = settingsItemHeight + 2 * settingsItemBorderWidth + 2 * settingsRowSpace;

    // config opener

    stASIOConfigLabel = buildSettingsLabel(
        stDriverSpecificSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*)"Config"
    );

    stASIOConfig = new Button();
    stDriverSpecificSection->addControl(stASIOConfig);

    stASIOConfig->setX(0.0);
    stASIOConfig->x += stASIOConfigLabel->width;
    stASIOConfig->setY(y);
    stASIOConfig->setWidth(0.5);
    stASIOConfig->setHeight(settingsItemHeight);
    stASIOConfig->paddingLeft = stLeftPadding;
    stASIOConfig->textAlignment = StringAlignment::LEFT;
    stASIOConfig->text = (char*)"Open";
    stASIOConfig->textLength = 4;
    stASIOConfig->fontSize = settingsItemFontSize;
    stASIOConfig->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    
    stASIOConfig->eMouseClick = &openASIOConfig;

    //
    // WASAPI specific settings
    //
    
    // share mode
    stWASAPIShareModeLabel = buildSettingsLabel(
        stDriverSpecificSection,
        y,
        settingsItemHeight,
        stLeftPadding,
        (char*) "ShareMode"
    );

    stWASAPIShareMode = new Checkbox();
    stDriverSpecificSection->addControl(stWASAPIShareMode);

    stWASAPIShareMode->setX(0.0);
    stWASAPIShareMode->x += stWASAPIShareModeLabel->width + stLeftPadding;
    stWASAPIShareMode->setY(y + (settingsItemHeight - checkboxHeight) / 2);
    stWASAPIShareMode->setWidth(0.5);
    stWASAPIShareMode->setHeight(checkboxHeight);
    stWASAPIShareMode->paddingLeft = stLeftPadding;
    stWASAPIShareMode->textAlignment = StringAlignment::LEFT;
    //stWASAPIShareMode->text = (char*)"Open";
    //stWASAPIShareMode->textLength = 4;
    stWASAPIShareMode->fontSize = settingsItemFontSize;
    stWASAPIShareMode->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    stWASAPIShareMode->setCheckType(Checkbox::CheckType::SQUARE);
    stWASAPIShareMode->checked = 1;
    stWASAPIShareMode->eChange = &shareModeWASAPIChange;

    toggleWASAPISettings(0);
    toggleASIOSettings(1);
    // 
    // 
    //
    selectedControlPanelMenu = pluginSelect;
    selectedControlPanelButton = cpPluginsButton;
    selectedControlPanelImage = cpPluginsButtonIcon;
    //



    // audio control header 
    // 
    const int audioHeaderHeight = 32;

    basicAudioControlHeader = new Control();
    rackPanel->addControl(basicAudioControlHeader);

    basicAudioControlHeader->setX(0.0);
    basicAudioControlHeader->setY(0.0);
    basicAudioControlHeader->setWidth(1.0);
    basicAudioControlHeader->setBorderWidth(4);
    basicAudioControlHeader->borderColor = MAIN_FRONT_COLOR;
    basicAudioControlHeader->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    basicAudioControlHeader->setHeight(audioHeaderHeight);

    // on/off switch
    audioDriverOnOffSwitch = new Image();
    basicAudioControlHeader->addControl(audioDriverOnOffSwitch);

    audioDriverOnOffSwitch->setPadding(4);
    audioDriverOnOffSwitch->setX(1.0);
    audioDriverOnOffSwitch->setY(0.0);
    audioDriverOnOffSwitch->setHeight(1.0);
    audioDriverOnOffSwitch->setWidth(audioDriverOnOffSwitch->height);
    audioDriverOnOffSwitch->x -= audioDriverOnOffSwitch->width;
    audioDriverOnOffSwitch->setColor(OFF_COLOR, ON_COLOR);
    audioDriverOnOffSwitch->drawType = ImageDrawType::ONE_COLOR;
    audioDriverOnOffSwitch->cursor = Cursor::POINTER;
    audioDriverOnOffSwitch->setImage((Render::Bitmap*) Resources::powerButtonOff);
    audioDriverOnOffSwitch->selected = 0;

    audioDriverOnOffSwitch->eMouseClick = &audioDriverOnOffSwitchClick;

    // save preset button
    Button* savePresetButton = new Button();
    basicAudioControlHeader->addControl(savePresetButton);

    //savePresetButton->setPadding(2);
    savePresetButton->setX(0.0);
    savePresetButton->x += 4;
    savePresetButton->setY(0.0);
    savePresetButton->y += 4;
    savePresetButton->setHeight(1.0);
    savePresetButton->height -= 8;
    savePresetButton->setWidth(10 * 6);
    savePresetButton->width -= 8;
    savePresetButton->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
    savePresetButton->text = (char*) "Save";
    savePresetButton->textLength = 4;
    savePresetButton->fontSize = 10;

    savePresetButton->eMouseClick = &savePreset;

    // save preset button
    Button* saveNewPresetButton = new Button();
    basicAudioControlHeader->addControl(saveNewPresetButton);

    //savePresetButton->setPadding(2);
    saveNewPresetButton->x = savePresetButton->x + savePresetButton->width + 4;
    saveNewPresetButton->setY(0.0);
    saveNewPresetButton->y += 4;
    saveNewPresetButton->setHeight(1.0);
    saveNewPresetButton->height -= 8;
    saveNewPresetButton->setWidth(10 * 9);
    saveNewPresetButton->width -= 8;
    saveNewPresetButton->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
    saveNewPresetButton->text = (char*) "Save As";
    saveNewPresetButton->textLength = 7;
    saveNewPresetButton->fontSize = 10;

    saveNewPresetButton->eMouseClick = &showSavePresetAsPopup;

    presetLabel = new Control();
    basicAudioControlHeader->addControl(presetLabel);

    //savePresetButton->setPadding(2);
    presetLabel->x = saveNewPresetButton->x + saveNewPresetButton->width + 8;
    presetLabel->setY(0.0);
    presetLabel->y += 4;
    presetLabel->setHeight(1.0);
    presetLabel->height -= 8;
    presetLabel->setWidth(audioDriverOnOffSwitch->x - presetLabel->x);
    presetLabel->width -= 8;
    presetLabel->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    presetLabel->textAlignment = StringAlignment::LEFT;
    presetLabel->fontSize = 8;

    // plugin container, that will contain all active / selected plugins
    pluginContainer = new PluginContainer();
    rackPanel->addControl(pluginContainer);

    pluginContainer->setY(basicAudioControlHeader->y + basicAudioControlHeader->height);
    pluginContainer->setX(0.0);
    pluginContainer->setWidth(1.0);
    pluginContainer->borderColor = MAIN_FRONT_COLOR;
    pluginContainer->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    pluginContainer->setBorderWidth(4);
    pluginContainer->setPadding(4);
    pluginContainer->eDrop = &pluginContainerDrop;
    pluginContainer->droppable = 1;
    pluginContainer->setHeight(1.0);
    pluginContainer->height = pluginContainer->height - audioHeaderHeight;

    int pluginCount = 0;
    pluginContainer->plugins = AudioDriver::getPlugins(&pluginCount);
    pluginContainer->setPluginCount(pluginCount);
    pluginContainer->samplePlugins = AudioDriver::getSamplePlugins();

    
    
    
    //

    savePresetAsPopup = new Control();
    window->addControl(savePresetAsPopup);

    savePresetAsPopup->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    savePresetAsPopup->setWidth(0.3);
    savePresetAsPopup->setHeight(0.3);
    savePresetAsPopup->setX(0.5);
    savePresetAsPopup->x -= savePresetAsPopup->width / 2;
    savePresetAsPopup->setY(0.5);
    savePresetAsPopup->y -= savePresetAsPopup->height / 2;
    savePresetAsPopup->visible = 0;

    Control* savePresetAsPopupHeader = new Control();
    savePresetAsPopup->addControl(savePresetAsPopupHeader);

    savePresetAsPopupHeader->setColor(Color::BLACK, Color::WHITE);
    savePresetAsPopupHeader->setWidth(1.0);
    savePresetAsPopupHeader->setX(0.0);
    savePresetAsPopupHeader->setHeight(40);
    savePresetAsPopupHeader->setY(0.0);
    savePresetAsPopupHeader->paddingLeft = 10;
    savePresetAsPopupHeader->setText((char*)"Pick the filename", 17);
    savePresetAsPopupHeader->fontSize = 8;
    savePresetAsPopupHeader->textAlignment = StringAlignment::LEFT;

    Control* savePresetAsPopupHeaderCloseButton = new Control();
    savePresetAsPopupHeader->addControl(savePresetAsPopupHeaderCloseButton);

    savePresetAsPopupHeaderCloseButton->setColor(Color::BLACK, Color::WHITE);
    savePresetAsPopupHeaderCloseButton->setWidth(40);
    savePresetAsPopupHeaderCloseButton->setX(1.0);
    savePresetAsPopupHeaderCloseButton->x -= 40;
    savePresetAsPopupHeaderCloseButton->setHeight(40);
    savePresetAsPopupHeaderCloseButton->setY(0.0);
    savePresetAsPopupHeaderCloseButton->paddingRight = 10;
    savePresetAsPopupHeaderCloseButton->setText((char*)"X", 1);
    savePresetAsPopupHeaderCloseButton->fontSize = 8;
    savePresetAsPopupHeaderCloseButton->textAlignment = StringAlignment::RIGHT;
    savePresetAsPopupHeaderCloseButton->cursor = Cursor::POINTER;

    savePresetAsPopupHeaderCloseButton->eMouseClick = &hideSavePresetAsPopup;


    Control* savePresetAsPopupBody = new Control();
    savePresetAsPopup->addControl(savePresetAsPopupBody);

    savePresetAsPopupBody->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    savePresetAsPopupBody->setWidth(1.0);
    savePresetAsPopupBody->setX(0.0);
    savePresetAsPopupBody->height = savePresetAsPopup->height - savePresetAsPopupHeader->height;
    savePresetAsPopupBody->y = savePresetAsPopupHeader->y + savePresetAsPopupHeader->height;
    savePresetAsPopupBody->setBorderWidth(4);
    savePresetAsPopupBody->borderColor = MAIN_FRONT_COLOR;

    Button* savePresetAsPopupSubmitButton = new Button();
    savePresetAsPopupBody->addControl(savePresetAsPopupSubmitButton);

    savePresetAsPopupSubmitButton->setColor(MAIN_BACK_COLOR, MAIN_FRONT_COLOR);
    savePresetAsPopupSubmitButton->setWidth(1.0);
    savePresetAsPopupSubmitButton->setHeight(30);
    savePresetAsPopupSubmitButton->setX(0.0);
    savePresetAsPopupSubmitButton->setY(1.0);
    savePresetAsPopupSubmitButton->y -= savePresetAsPopupSubmitButton->height;
    savePresetAsPopupSubmitButton->fontSize = 10;
    savePresetAsPopupSubmitButton->setText((char*)"Submit", 6);

    savePresetAsPopupSubmitButton->eMouseClick = &savePresetAs;

    const int svInpHeight = 14 + 8;
    const int svSpace = (savePresetAsPopupBody->height - savePresetAsPopupSubmitButton->height - 2 * svInpHeight) / 3;

    savePresetAsPopupNameInput = new Input();
    savePresetAsPopupBody->addControl(savePresetAsPopupNameInput);

    savePresetAsPopupNameInput->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    savePresetAsPopupNameInput->setWidth(0.8);
    savePresetAsPopupNameInput->height = svInpHeight;
    savePresetAsPopupNameInput->setX(0.5);
    savePresetAsPopupNameInput->x -= savePresetAsPopupNameInput->width / 2;
    savePresetAsPopupNameInput->setY(0.0);
    savePresetAsPopupNameInput->y += svSpace; // savePresetAsPopupNameInput->height / 2;
    savePresetAsPopupNameInput->fontSize = 10;
    savePresetAsPopupNameInput->borderBottomWidth = 2;
    savePresetAsPopupNameInput->paddingTop = 8;
    savePresetAsPopupNameInput->setBorderTitle((char*) "Preset Name", 11);
    savePresetAsPopupNameInput->borderTitleFontSize = 8;
    savePresetAsPopupNameInput->textAlignment = StringAlignment::LEFT;
    savePresetAsPopupNameInput->borderColor = MAIN_FRONT_COLOR;
    savePresetAsPopupNameInput->textCursorColor = MAIN_HIGHLIGHT_COLOR;
    savePresetAsPopupNameInput->cursor = Cursor::POINTER;

    savePresetAsPopupFileInput = new Input();
    savePresetAsPopupBody->addControl(savePresetAsPopupFileInput);

    savePresetAsPopupFileInput->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);
    savePresetAsPopupFileInput->setWidth(0.8);
    savePresetAsPopupFileInput->height = svInpHeight;
    savePresetAsPopupFileInput->setX(0.5);
    savePresetAsPopupFileInput->x -= savePresetAsPopupFileInput->width / 2;
    savePresetAsPopupFileInput->setY(0.0);
    savePresetAsPopupFileInput->y += svInpHeight + 2 * svSpace;
    savePresetAsPopupFileInput->fontSize = 10;
    savePresetAsPopupFileInput->borderBottomWidth = 2;
    savePresetAsPopupFileInput->paddingTop = 8;
    savePresetAsPopupFileInput->setBorderTitle((char*) "Filename", 8);
    savePresetAsPopupFileInput->borderTitleFontSize = 8;
    savePresetAsPopupFileInput->textAlignment = StringAlignment::LEFT;
    savePresetAsPopupFileInput->borderColor = MAIN_FRONT_COLOR;
    savePresetAsPopupFileInput->textCursorColor = MAIN_HIGHLIGHT_COLOR;
    savePresetAsPopupFileInput->cursor = Cursor::POINTER;

    return 0;

}

Control* buildSettingsLabel(Control* parent, int y, int height, int leftPadding, char* text) {

    Control* label = new Control();
    parent->addControl(label);

    label->setX(0.0);
    label->setY(y);
    label->setWidth(0.5);
    label->setHeight(height);
    label->paddingLeft = leftPadding;
    label->textAlignment = StringAlignment::LEFT;
    label->textLength = strlen(text);
    label->text = (char*) text;
    label->fontSize = 12;
    label->visible = 1;
    label->setColor(MAIN_FRONT_COLOR, MAIN_BACK_COLOR);

    return label;

}

void setDevices(AudioDriver::Device** devices, const int itemsCount) {

    char** items = (char**) malloc(sizeof(char*) * itemsCount);
    if (items == NULL) return;

    for (int i = 0; i < itemsCount; i++) {
        items[i] = devices[i]->name;
    }

    stDeviceSelect->insertItems(items, itemsCount);

}

void setChannel(AudioDriver::Channel channel, int io, int state) {

    switch (channel) {

        case (AudioDriver::CHANNEL_1): {
            
            if (io) {
                stInLeftChannel->checked = state;
            } else {
                stOutLeftChannel->checked = state;
            }

            break;
        }

        case (AudioDriver::CHANNEL_2): {
            
            if (io) {
                stInRightChannel->checked = state;
            } else {
                stOutRightChannel->checked = state;
            }

            break;
        }

    };

}

void setPresets(std::vector<Preset*> presets) {

    const int count = presets.size();
    for (int i = 0; i < count; i++) {
        presetSelectMenu->addItem(presets[i]->name);
    }

    presetSelectMenu->setItemHeight();

}

void pluginsClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

}

void pluginMenuDragStart(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

   // OutputDebugStringA("DRAG START\n");

}

void pluginMenuDragEnd(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    //OutputDebugStringA("DRAG END\n");

}

void pluginMenuDrag(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    //OutputDebugStringA("DRAG\n");

}

void pluginContainerDrop(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    //OutputDebugStringA("DROP\n");

}

void settingsClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    //char buffer[256];
    //sprintf(buffer, "PARAM_VAL: %i\n", paramB);

    //OutputDebugStringA(buffer);

}

void presetsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    selectControlMenuButton((Button*)source, presetSelect, cpPresetsButtonIcon);

}

void pluginsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    selectControlMenuButton((Button*)source, pluginSelect, cpPluginsButtonIcon);

}

void settingsButtonClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    selectControlMenuButton((Button*)source, settingsMenu, cpSettingsButtonIcon);

}

void selectControlMenuButton(Button* button, Control* menu, Image* image) {

    if (menu == NULL) return;

    if (selectedControlPanelButton != NULL && selectedControlPanelButton != button) selectedControlPanelButton->unselect();
    selectedControlPanelButton = button;
    selectedControlPanelButton->select();
    
    if (selectedControlPanelImage != NULL && selectedControlPanelImage != image) selectedControlPanelImage->unselect();
    selectedControlPanelImage = image;
    selectedControlPanelImage->select();

    if (selectedControlPanelMenu != NULL && selectedControlPanelMenu != menu) selectedControlPanelMenu->hide();
    selectedControlPanelMenu = menu;
    selectedControlPanelMenu->show(1);

}

void changeAudioDriver(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    const int idx = paramA;
    if (idx < 0 || idx >= AudioDriver::AD_COUNT) return;
        
    audioDriverOnOffSwitch->selected = 0;

    AudioDriver::Driver driver = (AudioDriver::Driver) idx;
    AudioDriver::select(driver);

    devices = AudioDriver::getDevices(&deviceCount);
    setDevices(devices, deviceCount);

    if (devices == NULL || deviceCount <= 0) {
        // cannot init driver, no supported device

        Utils::showError("Cannot init driver, no supported device!");
        
    } else {

        /*
        Config::setAudioDevice(Config::audioDeviceId >= deviceCount ? 0 : Config::audioDeviceId);
        AudioDriver::info.device = devices[Config::audioDeviceId];

        AudioDriver::info.channelIn = (Config::leftChannelIn ? AudioDriver::CHANNEL_1 : 0) |
            (Config::rightChannelIn ? AudioDriver::CHANNEL_2 : 0);

        AudioDriver::info.channelOut = (Config::leftChannelOut ? AudioDriver::CHANNEL_1 : 0) |
            (Config::rightChannelOut ? AudioDriver::CHANNEL_2 : 0);
        */

        AudioDriver::init(&AudioDriver::info);
        
    }

    if (toggleLastSettings != NULL) toggleLastSettings(0);
    switch (idx) {

        case AudioDriver::AD_ASIO: {
            
            toggleASIOSettings(1);
            toggleLastSettings = &toggleASIOSettings;

            break;

        }

        case AudioDriver::AD_WASAPI: {
            
            toggleWASAPISettings(1);
            toggleLastSettings = &toggleWASAPISettings;

            break;

        }

        case AudioDriver::AD_LEGACY: {

            toggleLastSettings = NULL;

            break;

        }

        case AudioDriver::AD_FILE: {
            
            toggleFileSettings(1);
            toggleLastSettings = &toggleFileSettings;

            break;

        }
    
    }

}

void stUIResizeTypeChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    const int idx = paramA;
    if (idx < 0 || idx >= 2) return; // change 2
    
    setResizeType((ResizeType::ResizeType)idx);

}

void setResizeType(ResizeType::ResizeType type) {

    switch (type) {

        case ResizeType::NONE:

            if (!Config::windowMaximize) System::blockMaximize();
            System::blockResize();
            System::setWindowSize(Config::renderWidth, Config::renderHeight); // change

            break;

        case ResizeType::SCALE:

            System::allowMaximize();
            System::allowResize();

            break;

    }

}

void toggleASIOSettings(int state) {

    stDriverSpecificSection->visible = state;

    stASIOConfig->visible = state;
    stASIOConfigLabel->visible = state;

    if (state) {
        stDriverSpecificSection->height = ASIOSettingsHeight;
        // stASIOConfigLabel->visible = state;
    }

}

void toggleWASAPISettings(int state) {

    stDriverSpecificSection->visible = state;

    stWASAPIShareMode->visible = state;
    stWASAPIShareModeLabel->visible = state;

    if (state) {
        stDriverSpecificSection->height = WASAPISettingsHeight;
        // stASIOConfigLabel->visible = state;
    }

}

void toggleFileSettings(int state) {

    stDriverSpecificSection->visible = 0;

}

void changeAudioDevice(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    const int idx = paramA;
    if (idx >= 0 || idx < deviceCount) {
        AudioDriver::info.device = devices[idx];
        AudioDriver::init(&AudioDriver::info);
    }

}

void stInLeftChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
    AudioDriver::info.channelIn ^= AudioDriver::CHANNEL_1;
    AudioDriver::setChannels(&AudioDriver::info);
}

void stInRightChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
    AudioDriver::info.channelIn ^= AudioDriver::CHANNEL_2;
    AudioDriver::setChannels(&AudioDriver::info);
}

void stOutLeftChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
    AudioDriver::info.channelOut ^= AudioDriver::CHANNEL_1;
    AudioDriver::setChannels(&AudioDriver::info);
}

void stOutRightChannelChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
    AudioDriver::info.channelOut ^= AudioDriver::CHANNEL_2;
    AudioDriver::setChannels(&AudioDriver::info);
}

void audioDriverOnOffSwitchClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {
    
    Image* src = (Image*) source;
    src->selected = !src->selected;

    const int on = src->selected;
    if (on) {
        AudioDriver::start();
    } else {
        AudioDriver::stop();
    }

    Render::redraw();

}

void openASIOConfig(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    AudioDriver::openExternalConfig();

}

void pluginSelectDblClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    SelectMenu* src = (SelectMenu*)source;
    const int idx = src->getPrimeIndex(); // check why hover idx could be more than actual item count

    if (idx >= plugins.size() || idx < 0) return;

    IPlugin* selectedPlugin = plugins[idx];
    AudioDriver::addPlugin(selectedPlugin);

    pluginContainer->plugins = AudioDriver::getPlugins();
    pluginContainer->samplePlugins = AudioDriver::getSamplePlugins();
    pluginContainer->setPluginCount(AudioDriver::getPluginCount());

    Render::redraw();

}

void savePreset(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    if (selectedPreset == NULL) {
        savePresetAsPopup->visible = 1;
        Render::redraw();
        //savePresetAs(source, paramA, paramB);
        return;
    }

    if (selectedPreset->replacePlugins(AudioDriver::getPlugins(), AudioDriver::getSamplePlugins(), AudioDriver::getPluginCount()) != 0) {
        Utils::showError("Cannot Save File!\nReplacement Failure!");
        return;
    }

    if (selectedPreset->save() != 0) {
        Utils::showError("Cannot Save File!");
        return;
    }

}

void savePresetAs(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    // firstly save rack as new preset
    Preset* newPreset = new Preset();
    
    char* const name = savePresetAsPopupNameInput->text;
    const int nameLen = savePresetAsPopupNameInput->textLength;

    char filename[256];
    savePresetAsPopupNameInput->text[savePresetAsPopupNameInput->textLength] = '\0';
    strcpy(filename, PRESET_FOLDER);
    strcpy(filename + strlen(PRESET_FOLDER), savePresetAsPopupNameInput->text);
    filename[strlen(PRESET_FOLDER) + savePresetAsPopupNameInput->textLength] = '.';
    strcpy(filename + strlen(PRESET_FOLDER) + 1 + savePresetAsPopupNameInput->textLength, PRESET_EXTENSION);
    const int filenameLen = savePresetAsPopupNameInput->textLength + strlen(PRESET_FOLDER) + 1 + strlen(PRESET_EXTENSION);
    
    newPreset->name = (char*) malloc(sizeof(char) * nameLen);
    if (newPreset == NULL) {
        goto exit;
    }
    
    strncpy(newPreset->name, name, nameLen);
    newPreset->nameLen = nameLen;
    
    if (newPreset->replacePlugins(AudioDriver::getPlugins(), AudioDriver::getSamplePlugins(), AudioDriver::getPluginCount()) != 0) {
        Utils::showError("Cannot Save File!\nReplacement Failure!");
        return;
    }

    // and then export to file
    if (newPreset->saveAs(filename)) {
        // probably invalid filename
        Utils::showError("Cannot create file, check if vaild filename was entered!");
        return;
    }

exit:
    presets.push_back(newPreset);
    presetSelectMenu->addItem(newPreset->name, newPreset->nameLen);
    presetSelectMenu->setItemHeight();
    savePresetAsPopup->visible = 0;
    Render::redraw();

}

void presetSelectDblClick(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    SelectMenu* src = (SelectMenu*) source;
    const int idx = src->getPrimeIndex();

    // if (idx >= plugins.size() || idx < 0) return;
    openPreset(presets[idx]);

    Render::redraw();
    
}

void showSavePresetAsPopup(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    savePresetAsPopup->visible = 1;
    Render::redraw();

}

void hideSavePresetAsPopup(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    savePresetAsPopup->visible = 0;
    Render::redraw();

}

void shareModeWASAPIChange(Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) {

    AudioIOWASAPI* audioIO = (AudioIOWASAPI*) AudioDriver::getInstance();
    audioIO->toggleMode();

}

void openPreset(Preset* preset) {

    closePreset();

    selectedPreset = preset;

    IPlugin** plugins = preset->plugins;
    const int pluginCount = preset->pluginCount;
    
    for (int i = 0; i < pluginCount; i++) {

        AudioDriver::addPlugin(plugins[i]);
        pluginContainer->plugins = AudioDriver::getPlugins();
        pluginContainer->samplePlugins = AudioDriver::getSamplePlugins();
        pluginContainer->setPluginCount(AudioDriver::getPluginCount());
        pluginContainer->setPluginState(i, preset->states[i].state);

        PluginUIHandler* const uihnd = pluginContainer->plugins[i]->uihnd;
        PluginControl** const ctrls = uihnd->controls;
        const int ctrlsCount = uihnd->controlCount - 1;
        const int valuesCount = preset->controls[i].count;

        const int count = (ctrlsCount < valuesCount) ? ctrlsCount : valuesCount;
        for (int j = 0; j < count; j++) {
            Plugin::setValue(ctrls[j + 1], preset->controls[i].values[j]);
        }

    }

    presetLabel->text = selectedPreset->name;
    presetLabel->textLength = selectedPreset->nameLen;

}

void clearRack() {
    
    AudioDriver::removeAll();
    pluginContainer->removeAll();

}

void closePreset() {

    clearRack();

    presetLabel->text = NULL;
    presetLabel->textLength = 0;

}
