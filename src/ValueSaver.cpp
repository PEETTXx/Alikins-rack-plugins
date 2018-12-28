#include "dsp/digital.hpp"
#include "alikins.hpp"

#define VALUE_COUNT 8
#define CLOSE_ENOUGH 0.01f

struct ValueSaver : Module {
    enum ParamIds {
        ENUMS(VALUE_PARAM, VALUE_COUNT),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(VALUE_INPUT, VALUE_COUNT),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(VALUE_OUTPUT, VALUE_COUNT),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ValueSaver() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    float values[VALUE_COUNT] = {0.0f};
    float prevInputs[VALUE_COUNT] = {0.0f};

    bool initialized = false;

    bool changingInputs[VALUE_COUNT] = {};
    bool initializedInputs[VALUE_COUNT] = {};
    SchmittTrigger valueUpTrigger[VALUE_COUNT];
    SchmittTrigger valueDownTrigger[VALUE_COUNT];

};

void ValueSaver::step()
{
    // states:
    //  - active inputs, meaningful current input -> output
    //  - active inputs,
    //  - in active inputs, meaningful 'saved' input -> output
    //  - in active inputs, default/unset value -> output
    //
    for (int i = 0; i < VALUE_COUNT; i++) {
        // if (i == 0) { debug("A changingInputs[%d]: %d", i, changingInputs[i]);}

        // process(rescale(in, low, high, 0.f, 1.f))
        /*
        float down = rescale(values[i], 0.0f, -CLOSE_ENOUGH, 0.0f, 1.f);
        float up = rescale(values[i], 0.0f, CLOSE_ENOUGH, 0.0f, 1.f);
        if (valueUpTrigger[i].process(up)) {
            debug("valueUpTrigger[%d] triggered value: %f %f", i, values[i], up);
        }
        if (valueDownTrigger[i].process(down)) {
            debug("valueDownTrigger[%d] triggered value: %f %f", i, values[i], down);
        }
        */

        // Just output the "saved" value if no active input
        // ... if it is changed from the initializedValue (as loaded from json) enough
        if (!inputs[VALUE_INPUT + i].active) {
            outputs[VALUE_OUTPUT + i].value = prevInputs[i];
            /*
            // prevInputs[i] = inputs[i].value;
            outputs[VALUE_OUTPUT + i].value = values[i];
            prevInputs[i] = values[i];
            continue;
            */
        } else {
            // ACTIVE INPUTS
            if (isNear(inputs[VALUE_INPUT + i].value, 0.0f)) {
                // active input put it is 0.0f, like a midi-1 on startup that hasn't sent any signal yet
                debug("[%d] ACTIVE but input is ~0.0f, prevInputs[%d]=%f", i, i, prevInputs[i]);
                values[i] = prevInputs[i];
                outputs[VALUE_OUTPUT + i].value = values[i];
            } else {
                // prevInputs[i] = inputs[i].value;
                // input value copied to output value and stored in values[]
                values[i] = inputs[VALUE_INPUT + i].value;
                outputs[VALUE_OUTPUT + i].value = values[i];
                // debug("[%d] copying input %f to output, prevInput: %f", i, values[i], prevInputs[i]);
                prevInputs[i] = values[i];
                continue;
            }
        }
        // active inputs
        // prevInputs[i] = values[i];
        // trigger?
        /*
        if (changingInputs[i]) {
            debug("changingInpputs[%d] changing value %f to input.value %f", i, values[i], inputs[VALUE_INPUT + i].value);
            values[i] = inputs[VALUE_INPUT + i].value;
            changingInputs[i] = false;
        }
        */

       /*
        if (fabs(prevInputs[i] - values[i]) > CLOSE_ENOUGH) {
            changingInputs[i] = true;
            // values[i] = inputs[VALUE_INPUT + i].value;
            if (i == 3) {
                debug("B changingInputs[%d]=%d: prevInputs[%d]=%f input[%d]=%f values[%d]: %f",
                 i, changingInputs[i], i, prevInputs[i], i, inputs[i].value, i, values[i]);
            }
            // prevInputs[i] = inputs[VALUE_INPUT + i].value;
            // outputs[VALUE_OUTPUT + i].value = values[i];
            // continue;
        }
        */
       /*
        prevInputs[i] = values[i];
        values[i] = inputs[VALUE_INPUT + i].value;
        outputs[VALUE_OUTPUT + i].value = values[i];
        */
        // outputs[VALUE_OUTPUT + i].value = values[i];

    }
}

json_t* ValueSaver::toJson() {
    json_t *rootJ = json_object();

    // json_object_set_new(rootJ, "valueCount", json_integer(inputRange));
    json_t *valuesJ = json_array();
    for (int i = 0; i < VALUE_COUNT; i++)
    {
        // debug("toJson current values[%d]: %f", i, values[i]);
        json_t *valueJ = json_real(values[i]);
        json_array_append_new(valuesJ, valueJ);
    }
    json_object_set_new(rootJ, "values", valuesJ);

    return rootJ;
}

void ValueSaver::fromJson(json_t *rootJ) {
    json_t *valuesJ = json_object_get(rootJ, "values");
    float savedInput;
    if (valuesJ)
    {
        for (int i = 0; i < VALUE_COUNT; i++)
        {
            // debug("fromJson i: %d", i);
            debug("current values[%d]: %f", i, values[i]);
            json_t *valueJ = json_array_get(valuesJ, i);
            if (valueJ) {
                debug("fromJson values[%d] value: %f", i, json_number_value(valueJ));
                savedInput = json_number_value(valueJ);
                prevInputs[i] = savedInput;
                initializedInputs[i] = savedInput;
                //inputs[i].value = savedInput;
                values[i] = savedInput;
                changingInputs[i] = false;
            }
        }
    }
    debug("INITIALIZED!");
    initialized = true;

}

struct ValueSaverWidget : ModuleWidget {
    ValueSaverWidget(ValueSaver *module);
};

ValueSaverWidget::ValueSaverWidget(ValueSaver *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ValueSaverPanel.svg")));

    float y_pos = 2.0f;

    for (int i = 0; i < VALUE_COUNT; i++) {
        float x_pos = 4.0f;

        addInput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          ValueSaver::VALUE_INPUT + i));

        x_pos += 30.0f;

        addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                           Port::OUTPUT,
                                           module,
                                           ValueSaver::VALUE_OUTPUT + i));

        y_pos += 35.0f;
    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelValueSaver = Model::create<ValueSaver, ValueSaverWidget>(
        "Alikins", "ValueSaver", "Value Saver", UTILITY_TAG);