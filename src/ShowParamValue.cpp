#include <stdio.h>
#include <string>
#include <vector>
#include <cstddef>
#include <array>
#include <map>
#include <math.h>
#include <float.h>
#include <sys/time.h>
#include <cxxabi.h>
#include <typeinfo>

#include "window.hpp"
#include <GLFW/glfw3.h>

#include "alikins.hpp"
#include "ui.hpp"

// #include "specificValueWidgets.hpp"

struct ShowParamValue : Module
{
    enum ParamIds
    {
        HOVERED_PARAM_VALUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        PARAM_VALUE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };


    ShowParamValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    float param_value;

};

void ShowParamValue::step()
{
    outputs[PARAM_VALUE_OUTPUT].value = param_value;
}

enum AdjustKey
{
    UP,
    DOWN,
    INITIAL
};

// TODO/FIXME: This is more or less adhoc TextField mixed with QuantityWidget
//             just inherit from both?
struct ParamFloatField : TextField
{
    ShowParamValue *module;
    float hovered_value;

    ParamFloatField(ShowParamValue *module);

    void setValue(float value);
    void onChange(EventChange &e) override;

    std::string paramValueToText(float param_value);

};

ParamFloatField::ParamFloatField(ShowParamValue *_module)
{
    module = _module;
}

std::string ParamFloatField::paramValueToText(float param_value){
    return stringf("%#.4g", param_value);
}

void ParamFloatField::setValue(float value) {
	this->hovered_value = value;
    this->module->param_value = value;
	EventChange e;
	onChange(e);
}


void ParamFloatField::onChange(EventChange &e) {
    // debug("ParamFloatField onChange  text=%s param=%f", text.c_str(),
    //    module->params[ShowParamValue::VALUE1_PARAM].value);
    std::string new_text = paramValueToText(hovered_value);
    setText(new_text);
}



struct ShowParamValueWidget : ModuleWidget
{
    ShowParamValueWidget(ShowParamValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    // float prev_volts = 0.0f;
    // float prev_input = 0.0f;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;
};

ShowParamValueWidget::ShowParamValueWidget(ShowParamValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/ShowParamValue.svg")));

    // TODO: widget with these children?
    float y_baseline = 45.0f;

    Vec param_value_field_size = Vec(70.0f, 22.0f);

    float x_pos = 10.0f;

    y_baseline = 38.0f;

    param_value_field = new ParamFloatField(module);
    param_value_field->box.pos = Vec(x_pos, y_baseline);
    param_value_field->box.size = param_value_field_size;

    addChild(param_value_field);

    Vec min_field_size = Vec(70.0f, 22.0f);

    y_baseline = 78.0f;
    min_field = new TextField();
    min_field->box.pos = Vec(x_pos, y_baseline);
    min_field->box.size = min_field_size;

    addChild(min_field);

    y_baseline = 118.0f;
    max_field = new TextField();
    max_field->box.pos = Vec(x_pos, y_baseline);
    max_field->box.size = min_field_size;

    addChild(max_field);

    y_baseline = 158.0f;
    default_field = new TextField();
    default_field->box.pos = Vec(x_pos, y_baseline);
    default_field->box.size = min_field_size;

    addChild(default_field);

    y_baseline = 198.0f;
    widget_type_field = new TextField();
    widget_type_field->box.pos = Vec(x_pos, y_baseline);
    widget_type_field->box.size = min_field_size;

    addChild(widget_type_field);

    float middle = box.size.x / 2.0f;
    float out_port_x = middle;

    y_baseline = box.size.y - 65.0f;
    Port *value_out_port = Port::create<PJ301MPort>(
        Vec(out_port_x, y_baseline),
        Port::OUTPUT,
        module,
        ShowParamValue::PARAM_VALUE_OUTPUT);

    outputs.push_back(value_out_port);
    value_out_port->box.pos = Vec(middle - value_out_port->box.size.x/2, y_baseline);

    addChild(value_out_port);

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void ShowParamValueWidget::step() {
    ModuleWidget::step();
    int status;
    char *realname;

/*
    if (prev_volts != module->params[ShowParamValue::VALUE1_PARAM].value ||
        prev_input != module->params[ShowParamValue::VALUE1_INPUT].value) {
            // debug("SpVWidget step - emitting EventChange / onChange prev_volts=%f param=%f",
            //     prev_volts, module->params[ShowParamValue::VALUE1_PARAM].value);
            prev_volts = module->params[ShowParamValue::VALUE1_PARAM].value;
            prev_input = module->params[ShowParamValue::VALUE1_INPUT].value;
            EventChange e;
		    onChange(e);
    }
    */

    if (gHoveredWidget) {

        //realname = abi::__cxa_demangle(typeid(*gHoveredWidget).name(), 0, 0, &status);
        //debug("gHoveredWidget type %s", realname);
        //free(realname);


        // TODO: if widget is an input/output, read it's value?
        ParamWidget *pwidget = dynamic_cast<ParamWidget*>(gHoveredWidget);
        if (pwidget) {
            /*
            realname = abi::__cxa_demangle(typeid(*pwidget).name(), 0, 0, &status);
            debug("pWidget type %s value: %f", realname, pwidget->value);
            free(realname);
            */
            // module.
            // trimpot->setValue(pwidget->value);
            param_value_field->setValue(pwidget->value);
            min_field->setText(stringf("%#.4g", pwidget->minValue));
            max_field->setText(stringf("%#.4g", pwidget->maxValue));
            default_field->setText(stringf("%#.4g", pwidget->defaultValue));
            widget_type_field->setText("Param");
        }
        Port *port = dynamic_cast<Port*>(gHoveredWidget);
        if (port) {
            // debug("port type %d", port->type);
            // debug("port id: %d", port->portId);
            if (port->type == port->INPUT) {
                // debug("port input value: %f", port->module->inputs[port->portId].value);
                param_value_field->setValue(port->module->inputs[port->portId].value);
                widget_type_field->setText("Input");
            }
            if (port->type == port->OUTPUT) {
                // debug("port output value: %f", port->module->outputs[port->portId].value);
                param_value_field->setValue(port->module->outputs[port->portId].value);
                widget_type_field->setText("Output");
            }
            // inputs/outputs dont have variable min/max, so just use the -10/+10 and
            // 0 for the default to get the point across.
            min_field->setText(stringf("%#.4g", -10.0f));
            max_field->setText(stringf("%#.4g", 10.0f));
            default_field->setText(stringf("%#.4g", 0.0f));
            // port->module->inputs[0].
        }
        // TODO: if a WireWidget, can we figure out it's in/out and current value? That would be cool.
        WireWidget *wire_widget = dynamic_cast<WireWidget*>(gHoveredWidget);
        if (wire_widget) {

        }
        //Widget *p = dynamic_cast<Widget*>(gHoveredWidget);
        // ModuleWidget *m = getAncestorOfType<ModuleWidget>();

    }

}

void ShowParamValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelShowParamValue = Model::create<ShowParamValue, ShowParamValueWidget>(
    "Alikins", "HoveredValue", "Show Value Of Hovered Widget", UTILITY_TAG);
