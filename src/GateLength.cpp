#include "dsp/digital.hpp"

#include "alikins.hpp"
#include "MsDisplayWidget.hpp"


struct GateLength : Module {
    enum ParamIds {
        GATE_LENGTH_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        TRIGGER_INPUT,
        GATE_LENGTH_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float gate_length = 2.34f;

    SchmittTrigger inputOnTrigger;

    PulseGenerator gateGenerator;

    GateLength() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    void step() override;

    void onReset() override {
    }

};

void GateLength::step() {
    // FIXME: add way to support >10.0s gate length
    gate_length = clamp(params[GATE_LENGTH_PARAM].value + inputs[GATE_LENGTH_INPUT].value, 0.0f, 10.0f);
    float sample_time = engineGetSampleTime();
    // float pulse_time = clamp(params[GATE_LENGTH_PARAM].value + inputs[GATE_LENGTH_INPUT].value, 0.0f, 10.0f);
    if (inputOnTrigger.process(inputs[TRIGGER_INPUT].value)) {
        debug("GL INPUT ON TRIGGER gate_length: %f", gate_length);
        gateGenerator.trigger(gate_length);
    }

    outputs[GATE_OUTPUT].value = gateGenerator.process(sample_time) ? 10.0f : 0.0f;
}

struct GateLengthWidget : ModuleWidget {
    GateLengthWidget(GateLength *module);
};

GateLengthWidget::GateLengthWidget(GateLength *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/GateLength.svg")));

    float y_pos = 4.0f;
    float x_pos = 4.0f;

    addInput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                Port::INPUT,
                module,
                GateLength::TRIGGER_INPUT));
    
    x_pos += 30.0f;
    
    MsDisplayWidget *gate_length_display = new MsDisplayWidget();
    gate_length_display->box.pos = Vec(x_pos, y_pos);
    gate_length_display->box.size = Vec(80, 24);
    gate_length_display->value = &module->gate_length;
    addChild(gate_length_display);
    
    // FIXME: use new sequential box hbox/vbox thing
    x_pos += 40.0f;
    x_pos += 50.0f;
    addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                Port::OUTPUT,
                module,
                GateLength::GATE_OUTPUT));

    x_pos = 36.0f;
    y_pos += 36.0f;

    addInput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                Port::INPUT,
                module,
                GateLength::GATE_LENGTH_INPUT));

    x_pos += 30.0f;
    addParam(ParamWidget::create<Trimpot>(Vec(x_pos, y_pos),
                module,
                GateLength::GATE_LENGTH_PARAM,
                0.0f, 10.0f, 0.1f));



    // addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    // addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    // addChild(Widget::create<ScrewSilver>(Vec(30, 365)));

}

Model *modelGateLength = Model::create<GateLength, GateLengthWidget>(
        "Alikins", "GateLength", "Gate Length", UTILITY_TAG);
