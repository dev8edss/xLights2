#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(DMXPanel)
 #include <wx/panel.h>
 class wxBitmapButton;
 class wxButton;
 class wxCheckBox;
 class wxFlexGridSizer;
 class wxNotebook;
 class wxNotebookEvent;
 class wxSlider;
 class wxStaticText;
 class wxTextCtrl;
 //*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class Model;

class DMXPanel: public xlEffectPanel
{
	std::list<Model*> GetActiveModels();

	public:

		DMXPanel(wxWindow* parent);
		virtual ~DMXPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(DMXPanel)
		BulkEditCheckBox* CheckBox_INVDMX10;
		BulkEditCheckBox* CheckBox_INVDMX11;
		BulkEditCheckBox* CheckBox_INVDMX12;
		BulkEditCheckBox* CheckBox_INVDMX13;
		BulkEditCheckBox* CheckBox_INVDMX14;
		BulkEditCheckBox* CheckBox_INVDMX15;
		BulkEditCheckBox* CheckBox_INVDMX16;
		BulkEditCheckBox* CheckBox_INVDMX17;
		BulkEditCheckBox* CheckBox_INVDMX18;
		BulkEditCheckBox* CheckBox_INVDMX19;
		BulkEditCheckBox* CheckBox_INVDMX1;
		BulkEditCheckBox* CheckBox_INVDMX20;
		BulkEditCheckBox* CheckBox_INVDMX21;
		BulkEditCheckBox* CheckBox_INVDMX22;
		BulkEditCheckBox* CheckBox_INVDMX23;
		BulkEditCheckBox* CheckBox_INVDMX24;
		BulkEditCheckBox* CheckBox_INVDMX25;
		BulkEditCheckBox* CheckBox_INVDMX26;
		BulkEditCheckBox* CheckBox_INVDMX27;
		BulkEditCheckBox* CheckBox_INVDMX28;
		BulkEditCheckBox* CheckBox_INVDMX29;
		BulkEditCheckBox* CheckBox_INVDMX2;
		BulkEditCheckBox* CheckBox_INVDMX30;
		BulkEditCheckBox* CheckBox_INVDMX31;
		BulkEditCheckBox* CheckBox_INVDMX32;
		BulkEditCheckBox* CheckBox_INVDMX33;
		BulkEditCheckBox* CheckBox_INVDMX34;
		BulkEditCheckBox* CheckBox_INVDMX35;
		BulkEditCheckBox* CheckBox_INVDMX36;
		BulkEditCheckBox* CheckBox_INVDMX37;
		BulkEditCheckBox* CheckBox_INVDMX38;
		BulkEditCheckBox* CheckBox_INVDMX39;
		BulkEditCheckBox* CheckBox_INVDMX3;
		BulkEditCheckBox* CheckBox_INVDMX40;
		BulkEditCheckBox* CheckBox_INVDMX41;
		BulkEditCheckBox* CheckBox_INVDMX42;
		BulkEditCheckBox* CheckBox_INVDMX43;
		BulkEditCheckBox* CheckBox_INVDMX44;
		BulkEditCheckBox* CheckBox_INVDMX45;
		BulkEditCheckBox* CheckBox_INVDMX46;
		BulkEditCheckBox* CheckBox_INVDMX47;
		BulkEditCheckBox* CheckBox_INVDMX48;
		BulkEditCheckBox* CheckBox_INVDMX4;
		BulkEditCheckBox* CheckBox_INVDMX5;
		BulkEditCheckBox* CheckBox_INVDMX6;
		BulkEditCheckBox* CheckBox_INVDMX7;
		BulkEditCheckBox* CheckBox_INVDMX8;
		BulkEditCheckBox* CheckBox_INVDMX9;
		BulkEditSlider* Slider_DMX10;
		BulkEditSlider* Slider_DMX11;
		BulkEditSlider* Slider_DMX12;
		BulkEditSlider* Slider_DMX13;
		BulkEditSlider* Slider_DMX14;
		BulkEditSlider* Slider_DMX15;
		BulkEditSlider* Slider_DMX16;
		BulkEditSlider* Slider_DMX17;
		BulkEditSlider* Slider_DMX18;
		BulkEditSlider* Slider_DMX19;
		BulkEditSlider* Slider_DMX1;
		BulkEditSlider* Slider_DMX20;
		BulkEditSlider* Slider_DMX21;
		BulkEditSlider* Slider_DMX22;
		BulkEditSlider* Slider_DMX23;
		BulkEditSlider* Slider_DMX24;
		BulkEditSlider* Slider_DMX25;
		BulkEditSlider* Slider_DMX26;
		BulkEditSlider* Slider_DMX27;
		BulkEditSlider* Slider_DMX28;
		BulkEditSlider* Slider_DMX29;
		BulkEditSlider* Slider_DMX2;
		BulkEditSlider* Slider_DMX30;
		BulkEditSlider* Slider_DMX31;
		BulkEditSlider* Slider_DMX32;
		BulkEditSlider* Slider_DMX33;
		BulkEditSlider* Slider_DMX34;
		BulkEditSlider* Slider_DMX35;
		BulkEditSlider* Slider_DMX36;
		BulkEditSlider* Slider_DMX37;
		BulkEditSlider* Slider_DMX38;
		BulkEditSlider* Slider_DMX39;
		BulkEditSlider* Slider_DMX3;
		BulkEditSlider* Slider_DMX40;
		BulkEditSlider* Slider_DMX41;
		BulkEditSlider* Slider_DMX42;
		BulkEditSlider* Slider_DMX43;
		BulkEditSlider* Slider_DMX44;
		BulkEditSlider* Slider_DMX45;
		BulkEditSlider* Slider_DMX46;
		BulkEditSlider* Slider_DMX47;
		BulkEditSlider* Slider_DMX48;
		BulkEditSlider* Slider_DMX4;
		BulkEditSlider* Slider_DMX5;
		BulkEditSlider* Slider_DMX6;
		BulkEditSlider* Slider_DMX7;
		BulkEditSlider* Slider_DMX8;
		BulkEditSlider* Slider_DMX9;
		BulkEditValueCurveButton* ValueCurve_DMX10;
		BulkEditValueCurveButton* ValueCurve_DMX11;
		BulkEditValueCurveButton* ValueCurve_DMX12;
		BulkEditValueCurveButton* ValueCurve_DMX13;
		BulkEditValueCurveButton* ValueCurve_DMX14;
		BulkEditValueCurveButton* ValueCurve_DMX15;
		BulkEditValueCurveButton* ValueCurve_DMX16;
		BulkEditValueCurveButton* ValueCurve_DMX17;
		BulkEditValueCurveButton* ValueCurve_DMX18;
		BulkEditValueCurveButton* ValueCurve_DMX19;
		BulkEditValueCurveButton* ValueCurve_DMX1;
		BulkEditValueCurveButton* ValueCurve_DMX20;
		BulkEditValueCurveButton* ValueCurve_DMX21;
		BulkEditValueCurveButton* ValueCurve_DMX22;
		BulkEditValueCurveButton* ValueCurve_DMX23;
		BulkEditValueCurveButton* ValueCurve_DMX24;
		BulkEditValueCurveButton* ValueCurve_DMX25;
		BulkEditValueCurveButton* ValueCurve_DMX26;
		BulkEditValueCurveButton* ValueCurve_DMX27;
		BulkEditValueCurveButton* ValueCurve_DMX28;
		BulkEditValueCurveButton* ValueCurve_DMX29;
		BulkEditValueCurveButton* ValueCurve_DMX2;
		BulkEditValueCurveButton* ValueCurve_DMX30;
		BulkEditValueCurveButton* ValueCurve_DMX31;
		BulkEditValueCurveButton* ValueCurve_DMX32;
		BulkEditValueCurveButton* ValueCurve_DMX33;
		BulkEditValueCurveButton* ValueCurve_DMX34;
		BulkEditValueCurveButton* ValueCurve_DMX35;
		BulkEditValueCurveButton* ValueCurve_DMX36;
		BulkEditValueCurveButton* ValueCurve_DMX37;
		BulkEditValueCurveButton* ValueCurve_DMX38;
		BulkEditValueCurveButton* ValueCurve_DMX39;
		BulkEditValueCurveButton* ValueCurve_DMX3;
		BulkEditValueCurveButton* ValueCurve_DMX40;
		BulkEditValueCurveButton* ValueCurve_DMX41;
		BulkEditValueCurveButton* ValueCurve_DMX42;
		BulkEditValueCurveButton* ValueCurve_DMX43;
		BulkEditValueCurveButton* ValueCurve_DMX44;
		BulkEditValueCurveButton* ValueCurve_DMX45;
		BulkEditValueCurveButton* ValueCurve_DMX46;
		BulkEditValueCurveButton* ValueCurve_DMX47;
		BulkEditValueCurveButton* ValueCurve_DMX48;
		BulkEditValueCurveButton* ValueCurve_DMX4;
		BulkEditValueCurveButton* ValueCurve_DMX5;
		BulkEditValueCurveButton* ValueCurve_DMX6;
		BulkEditValueCurveButton* ValueCurve_DMX7;
		BulkEditValueCurveButton* ValueCurve_DMX8;
		BulkEditValueCurveButton* ValueCurve_DMX9;
		wxButton* ButtonRemap;
		wxButton* Button_Load_State;
		wxButton* Button_SaveAsState;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxFlexGridSizer* FlexGridSizer_Panel1;
		wxFlexGridSizer* FlexGridSizer_Panel2;
		wxFlexGridSizer* FlexGridSizer_Panel3;
		wxNotebook* Notebook7;
		wxPanel* ChannelPanel1;
		wxPanel* ChannelPanel2;
		wxPanel* ChannelPanel3;
		wxStaticText* Label_DMX10;
		wxStaticText* Label_DMX11;
		wxStaticText* Label_DMX12;
		wxStaticText* Label_DMX13;
		wxStaticText* Label_DMX14;
		wxStaticText* Label_DMX15;
		wxStaticText* Label_DMX16;
		wxStaticText* Label_DMX17;
		wxStaticText* Label_DMX18;
		wxStaticText* Label_DMX19;
		wxStaticText* Label_DMX1;
		wxStaticText* Label_DMX20;
		wxStaticText* Label_DMX21;
		wxStaticText* Label_DMX22;
		wxStaticText* Label_DMX23;
		wxStaticText* Label_DMX24;
		wxStaticText* Label_DMX25;
		wxStaticText* Label_DMX26;
		wxStaticText* Label_DMX27;
		wxStaticText* Label_DMX28;
		wxStaticText* Label_DMX29;
		wxStaticText* Label_DMX2;
		wxStaticText* Label_DMX30;
		wxStaticText* Label_DMX31;
		wxStaticText* Label_DMX32;
		wxStaticText* Label_DMX33;
		wxStaticText* Label_DMX34;
		wxStaticText* Label_DMX35;
		wxStaticText* Label_DMX36;
		wxStaticText* Label_DMX37;
		wxStaticText* Label_DMX38;
		wxStaticText* Label_DMX39;
		wxStaticText* Label_DMX3;
		wxStaticText* Label_DMX40;
		wxStaticText* Label_DMX41;
		wxStaticText* Label_DMX42;
		wxStaticText* Label_DMX43;
		wxStaticText* Label_DMX44;
		wxStaticText* Label_DMX45;
		wxStaticText* Label_DMX46;
		wxStaticText* Label_DMX47;
		wxStaticText* Label_DMX48;
		wxStaticText* Label_DMX4;
		wxStaticText* Label_DMX5;
		wxStaticText* Label_DMX6;
		wxStaticText* Label_DMX7;
		wxStaticText* Label_DMX8;
		wxStaticText* Label_DMX9;
		//*)

	protected:

		//(*Identifiers(DMXPanel)
		static const wxWindowID ID_STATICTEXT_DMX1;
		static const wxWindowID ID_SLIDER_DMX1;
		static const wxWindowID ID_VALUECURVE_DMX1;
		static const wxWindowID IDD_TEXTCTRL_DMX1;
		static const wxWindowID ID_CHECKBOX_INVDMX1;
		static const wxWindowID ID_STATICTEXT_DMX2;
		static const wxWindowID ID_SLIDER_DMX2;
		static const wxWindowID ID_VALUECURVE_DMX2;
		static const wxWindowID IDD_TEXTCTRL_DMX2;
		static const wxWindowID ID_CHECKBOX_INVDMX2;
		static const wxWindowID ID_STATICTEXT_DMX3;
		static const wxWindowID ID_SLIDER_DMX3;
		static const wxWindowID ID_VALUECURVE_DMX3;
		static const wxWindowID IDD_TEXTCTRL_DMX3;
		static const wxWindowID ID_CHECKBOX_INVDMX3;
		static const wxWindowID ID_STATICTEXT_DMX4;
		static const wxWindowID ID_SLIDER_DMX4;
		static const wxWindowID ID_VALUECURVE_DMX4;
		static const wxWindowID IDD_TEXTCTRL_DMX4;
		static const wxWindowID ID_CHECKBOX_INVDMX4;
		static const wxWindowID ID_STATICTEXT_DMX5;
		static const wxWindowID ID_SLIDER_DMX5;
		static const wxWindowID ID_VALUECURVE_DMX5;
		static const wxWindowID IDD_TEXTCTRL_DMX5;
		static const wxWindowID ID_CHECKBOX_INVDMX5;
		static const wxWindowID ID_STATICTEXT_DMX6;
		static const wxWindowID ID_SLIDER_DMX6;
		static const wxWindowID ID_VALUECURVE_DMX6;
		static const wxWindowID IDD_TEXTCTRL_DMX6;
		static const wxWindowID ID_CHECKBOX_INVDMX6;
		static const wxWindowID ID_STATICTEXT_DMX7;
		static const wxWindowID ID_SLIDER_DMX7;
		static const wxWindowID ID_VALUECURVE_DMX7;
		static const wxWindowID IDD_TEXTCTRL_DMX7;
		static const wxWindowID ID_CHECKBOX_INVDMX7;
		static const wxWindowID ID_STATICTEXT_DMX8;
		static const wxWindowID ID_SLIDER_DMX8;
		static const wxWindowID ID_VALUECURVE_DMX8;
		static const wxWindowID IDD_TEXTCTRL_DMX8;
		static const wxWindowID ID_CHECKBOX_INVDMX8;
		static const wxWindowID ID_STATICTEXT_DMX9;
		static const wxWindowID ID_SLIDER_DMX9;
		static const wxWindowID ID_VALUECURVE_DMX9;
		static const wxWindowID IDD_TEXTCTRL_DMX9;
		static const wxWindowID ID_CHECKBOX_INVDMX9;
		static const wxWindowID ID_STATICTEXT_DMX10;
		static const wxWindowID ID_SLIDER_DMX10;
		static const wxWindowID ID_VALUECURVE_DMX10;
		static const wxWindowID IDD_TEXTCTRL_DMX10;
		static const wxWindowID ID_CHECKBOX_INVDMX10;
		static const wxWindowID ID_STATICTEXT_DMX11;
		static const wxWindowID ID_SLIDER_DMX11;
		static const wxWindowID ID_VALUECURVE_DMX11;
		static const wxWindowID IDD_TEXTCTRL_DMX11;
		static const wxWindowID ID_CHECKBOX_INVDMX11;
		static const wxWindowID ID_STATICTEXT_DMX12;
		static const wxWindowID ID_SLIDER_DMX12;
		static const wxWindowID ID_VALUECURVE_DMX12;
		static const wxWindowID IDD_TEXTCTRL_DMX12;
		static const wxWindowID ID_CHECKBOX_INVDMX12;
		static const wxWindowID ID_STATICTEXT_DMX13;
		static const wxWindowID ID_SLIDER_DMX13;
		static const wxWindowID ID_VALUECURVE_DMX13;
		static const wxWindowID IDD_TEXTCTRL_DMX13;
		static const wxWindowID ID_CHECKBOX_INVDMX13;
		static const wxWindowID ID_STATICTEXT_DMX14;
		static const wxWindowID ID_SLIDER_DMX14;
		static const wxWindowID ID_VALUECURVE_DMX14;
		static const wxWindowID IDD_TEXTCTRL_DMX14;
		static const wxWindowID ID_CHECKBOX_INVDMX14;
		static const wxWindowID ID_STATICTEXT_DMX15;
		static const wxWindowID ID_SLIDER_DMX15;
		static const wxWindowID ID_VALUECURVE_DMX15;
		static const wxWindowID IDD_TEXTCTRL_DMX15;
		static const wxWindowID ID_CHECKBOX_INVDMX15;
		static const wxWindowID ID_STATICTEXT_DMX16;
		static const wxWindowID ID_SLIDER_DMX16;
		static const wxWindowID ID_VALUECURVE_DMX16;
		static const wxWindowID IDD_TEXTCTRL_DMX16;
		static const wxWindowID ID_CHECKBOX_INVDMX16;
		static const wxWindowID ID_PANEL6;
		static const wxWindowID ID_STATICTEXT_DMX17;
		static const wxWindowID ID_SLIDER_DMX17;
		static const wxWindowID ID_VALUECURVE_DMX17;
		static const wxWindowID IDD_TEXTCTRL_DMX17;
		static const wxWindowID ID_CHECKBOX_INVDMX17;
		static const wxWindowID ID_STATICTEXT_DMX18;
		static const wxWindowID ID_SLIDER_DMX18;
		static const wxWindowID ID_VALUECURVE_DMX18;
		static const wxWindowID IDD_TEXTCTRL_DMX18;
		static const wxWindowID ID_CHECKBOX_INVDMX18;
		static const wxWindowID ID_STATICTEXT_DMX19;
		static const wxWindowID ID_SLIDER_DMX19;
		static const wxWindowID ID_VALUECURVE_DMX19;
		static const wxWindowID IDD_TEXTCTRL_DMX19;
		static const wxWindowID ID_CHECKBOX_INVDMX19;
		static const wxWindowID ID_STATICTEXT_DMX20;
		static const wxWindowID ID_SLIDER_DMX20;
		static const wxWindowID ID_VALUECURVE_DMX20;
		static const wxWindowID IDD_TEXTCTRL_DMX20;
		static const wxWindowID ID_CHECKBOX_INVDMX20;
		static const wxWindowID ID_STATICTEXT_DMX21;
		static const wxWindowID ID_SLIDER_DMX21;
		static const wxWindowID ID_VALUECURVE_DMX21;
		static const wxWindowID IDD_TEXTCTRL_DMX21;
		static const wxWindowID ID_CHECKBOX_INVDMX21;
		static const wxWindowID ID_STATICTEXT_DMX22;
		static const wxWindowID ID_SLIDER_DMX22;
		static const wxWindowID ID_VALUECURVE_DMX22;
		static const wxWindowID IDD_TEXTCTRL_DMX22;
		static const wxWindowID ID_CHECKBOX_INVDMX22;
		static const wxWindowID ID_STATICTEXT_DMX23;
		static const wxWindowID ID_SLIDER_DMX23;
		static const wxWindowID ID_VALUECURVE_DMX23;
		static const wxWindowID IDD_TEXTCTRL_DMX23;
		static const wxWindowID ID_CHECKBOX_INVDMX23;
		static const wxWindowID ID_STATICTEXT_DMX24;
		static const wxWindowID ID_SLIDER_DMX24;
		static const wxWindowID ID_VALUECURVE_DMX24;
		static const wxWindowID IDD_TEXTCTRL_DMX24;
		static const wxWindowID ID_CHECKBOX_INVDMX24;
		static const wxWindowID ID_STATICTEXT_DMX25;
		static const wxWindowID ID_SLIDER_DMX25;
		static const wxWindowID ID_VALUECURVE_DMX25;
		static const wxWindowID IDD_TEXTCTRL_DMX25;
		static const wxWindowID ID_CHECKBOX_INVDMX25;
		static const wxWindowID ID_STATICTEXT_DMX26;
		static const wxWindowID ID_SLIDER_DMX26;
		static const wxWindowID ID_VALUECURVE_DMX26;
		static const wxWindowID IDD_TEXTCTRL_DMX26;
		static const wxWindowID ID_CHECKBOX_INVDMX26;
		static const wxWindowID ID_STATICTEXT_DMX27;
		static const wxWindowID ID_SLIDER_DMX27;
		static const wxWindowID ID_VALUECURVE_DMX27;
		static const wxWindowID IDD_TEXTCTRL_DMX27;
		static const wxWindowID ID_CHECKBOX_INVDMX27;
		static const wxWindowID ID_STATICTEXT_DMX28;
		static const wxWindowID ID_SLIDER_DMX28;
		static const wxWindowID ID_VALUECURVE_DMX28;
		static const wxWindowID IDD_TEXTCTRL_DMX28;
		static const wxWindowID ID_CHECKBOX_INVDMX28;
		static const wxWindowID ID_STATICTEXT_DMX29;
		static const wxWindowID ID_SLIDER_DMX29;
		static const wxWindowID ID_VALUECURVE_DMX29;
		static const wxWindowID IDD_TEXTCTRL_DMX29;
		static const wxWindowID ID_CHECKBOX_INVDMX29;
		static const wxWindowID ID_STATICTEXT_DMX30;
		static const wxWindowID ID_SLIDER_DMX30;
		static const wxWindowID ID_VALUECURVE_DMX30;
		static const wxWindowID IDD_TEXTCTRL_DMX30;
		static const wxWindowID ID_CHECKBOX_INVDMX30;
		static const wxWindowID ID_STATICTEXT_DMX31;
		static const wxWindowID ID_SLIDER_DMX31;
		static const wxWindowID ID_VALUECURVE_DMX31;
		static const wxWindowID IDD_TEXTCTRL_DMX31;
		static const wxWindowID ID_CHECKBOX_INVDMX31;
		static const wxWindowID ID_STATICTEXT_DMX32;
		static const wxWindowID ID_SLIDER_DMX32;
		static const wxWindowID ID_VALUECURVE_DMX32;
		static const wxWindowID IDD_TEXTCTRL_DMX32;
		static const wxWindowID ID_CHECKBOX_INVDMX32;
		static const wxWindowID ID_PANEL28;
		static const wxWindowID ID_STATICTEXT_DMX33;
		static const wxWindowID ID_SLIDER_DMX33;
		static const wxWindowID ID_VALUECURVE_DMX33;
		static const wxWindowID IDD_TEXTCTRL_DMX33;
		static const wxWindowID ID_CHECKBOX_INVDMX33;
		static const wxWindowID ID_STATICTEXT_DMX34;
		static const wxWindowID ID_SLIDER_DMX34;
		static const wxWindowID ID_VALUECURVE_DMX34;
		static const wxWindowID IDD_TEXTCTRL_DMX34;
		static const wxWindowID ID_CHECKBOX_INVDMX34;
		static const wxWindowID ID_STATICTEXT_DMX35;
		static const wxWindowID ID_SLIDER_DMX35;
		static const wxWindowID ID_VALUECURVE_DMX35;
		static const wxWindowID IDD_TEXTCTRL_DMX35;
		static const wxWindowID ID_CHECKBOX_INVDMX35;
		static const wxWindowID ID_STATICTEXT_DMX36;
		static const wxWindowID ID_SLIDER_DMX36;
		static const wxWindowID ID_VALUECURVE_DMX36;
		static const wxWindowID IDD_TEXTCTRL_DMX36;
		static const wxWindowID ID_CHECKBOX_INVDMX36;
		static const wxWindowID ID_STATICTEXT_DMX37;
		static const wxWindowID ID_SLIDER_DMX37;
		static const wxWindowID ID_VALUECURVE_DMX37;
		static const wxWindowID IDD_TEXTCTRL_DMX37;
		static const wxWindowID ID_CHECKBOX_INVDMX37;
		static const wxWindowID ID_STATICTEXT_DMX38;
		static const wxWindowID ID_SLIDER_DMX38;
		static const wxWindowID ID_VALUECURVE_DMX38;
		static const wxWindowID IDD_TEXTCTRL_DMX38;
		static const wxWindowID ID_CHECKBOX_INVDMX38;
		static const wxWindowID ID_STATICTEXT_DMX39;
		static const wxWindowID ID_SLIDER_DMX39;
		static const wxWindowID ID_VALUECURVE_DMX39;
		static const wxWindowID IDD_TEXTCTRL_DMX39;
		static const wxWindowID ID_CHECKBOX_INVDMX39;
		static const wxWindowID ID_STATICTEXT_DMX40;
		static const wxWindowID ID_SLIDER_DMX40;
		static const wxWindowID ID_VALUECURVE_DMX40;
		static const wxWindowID IDD_TEXTCTRL_DMX40;
		static const wxWindowID ID_CHECKBOX_INVDMX40;
		static const wxWindowID ID_STATICTEXT_DMX41;
		static const wxWindowID ID_SLIDER_DMX41;
		static const wxWindowID ID_VALUECURVE_DMX41;
		static const wxWindowID IDD_TEXTCTRL_DMX41;
		static const wxWindowID ID_CHECKBOX_INVDMX41;
		static const wxWindowID ID_STATICTEXT_DMX42;
		static const wxWindowID ID_SLIDER_DMX42;
		static const wxWindowID ID_VALUECURVE_DMX42;
		static const wxWindowID IDD_TEXTCTRL_DMX42;
		static const wxWindowID ID_CHECKBOX_INVDMX42;
		static const wxWindowID ID_STATICTEXT_DMX43;
		static const wxWindowID ID_SLIDER_DMX43;
		static const wxWindowID ID_VALUECURVE_DMX43;
		static const wxWindowID IDD_TEXTCTRL_DMX43;
		static const wxWindowID ID_CHECKBOX_INVDMX43;
		static const wxWindowID ID_STATICTEXT_DMX44;
		static const wxWindowID ID_SLIDER_DMX44;
		static const wxWindowID ID_VALUECURVE_DMX44;
		static const wxWindowID IDD_TEXTCTRL_DMX44;
		static const wxWindowID ID_CHECKBOX_INVDMX44;
		static const wxWindowID ID_STATICTEXT_DMX45;
		static const wxWindowID ID_SLIDER_DMX45;
		static const wxWindowID ID_VALUECURVE_DMX45;
		static const wxWindowID IDD_TEXTCTRL_DMX45;
		static const wxWindowID ID_CHECKBOX_INVDMX45;
		static const wxWindowID ID_STATICTEXT_DMX46;
		static const wxWindowID ID_SLIDER_DMX46;
		static const wxWindowID ID_VALUECURVE_DMX46;
		static const wxWindowID IDD_TEXTCTRL_DMX46;
		static const wxWindowID ID_CHECKBOX_INVDMX46;
		static const wxWindowID ID_STATICTEXT_DMX47;
		static const wxWindowID ID_SLIDER_DMX47;
		static const wxWindowID ID_VALUECURVE_DMX47;
		static const wxWindowID IDD_TEXTCTRL_DMX47;
		static const wxWindowID ID_CHECKBOX_INVDMX47;
		static const wxWindowID ID_STATICTEXT_DMX48;
		static const wxWindowID ID_SLIDER_DMX48;
		static const wxWindowID ID_VALUECURVE_DMX48;
		static const wxWindowID IDD_TEXTCTRL_DMX48;
		static const wxWindowID ID_CHECKBOX_INVDMX48;
		static const wxWindowID ID_PANEL3;
		static const wxWindowID ID_NOTEBOOK1;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_BUTTON2;
		static const wxWindowID ID_BUTTON_LOAD_STATE;
		//*)

	public:

		//(*Handlers(DMXPanel)
		void OnButtonRemapClick(wxCommandEvent& event);
		void OnButton_SaveAsStateClick(wxCommandEvent& event);
		void OnButton_Load_StateClick(wxCommandEvent& event);
		void OnNotebook7PageChanged(wxNotebookEvent& event);
		//*)

        void OnButtonRemapRClick(wxCommandEvent& event);
        void OnChoicePopup(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};
