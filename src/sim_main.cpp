#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "animations/EggChickAnimation.h"
#include "animations/ValentineHeartbeat.h"
#include "animations/ValentineTwoHearts.h"
#include "animations/ValentineLoveLetter.h"
#include "animations/ValentineTextOnly.h"
#include "animations/ValentineTextChick.h"
#include "animations/Chick1Animation.h"
#include "animations/Mosq1Animation.h"
#include "display/SimulatedDisplay.h"

int main(int argc, char** argv) {
    std::srand(12345);

    SimulatedDisplay display(64, 64, 8);

    eggChickAnimation.init(&display);
    valentineHeartbeat.init(&display);
    valentineTwoHearts.init(&display);
    valentineLoveLetter.init(&display);
    valentineTextOnly.init(&display);
    valentineTextChick.init(&display);
    chick1Animation.init(&display);
    mosq1Animation.init(&display);

    std::string anim = "val4";
    if (argc > 1 && argv[1] != nullptr) {
        anim = argv[1];
    }

    std::string prefix;

    if (anim == "val1") {
        valentineHeartbeat.start();
        prefix = "val1";
    } else if (anim == "val2") {
        valentineTwoHearts.start();
        prefix = "val2";
    } else if (anim == "val3") {
        valentineLoveLetter.start();
        prefix = "val3";
    } else if (anim == "egg") {
        eggChickAnimation.start();
        prefix = "egg";
    } else if (anim == "val5") {
        valentineTextChick.start();
        prefix = "val5";
    } else if (anim == "chick1") {
        chick1Animation.start();
        prefix = "chick1";
    } else if (anim == "mosq1") {
        mosq1Animation.start();
        prefix = "mosq1";
    } else {
        valentineTextOnly.start();
        prefix = "val4";
    }

    std::filesystem::create_directories("sim_frames");

    const int totalFrames = 240;
    const int writeEvery = 3;

    for (int frame = 0; frame < totalFrames; frame++) {
        if (prefix == "val1") {
            valentineHeartbeat.update();
            valentineHeartbeat.render();
        } else if (prefix == "val2") {
            valentineTwoHearts.update();
            valentineTwoHearts.render();
        } else if (prefix == "val3") {
            valentineLoveLetter.update();
            valentineLoveLetter.render();
        } else if (prefix == "egg") {
            eggChickAnimation.update();
            eggChickAnimation.render();
        } else if (prefix == "val5") {
            valentineTextChick.update();
            valentineTextChick.render();
        } else if (prefix == "chick1") {
            chick1Animation.update();
            chick1Animation.render();
        } else if (prefix == "mosq1") {
            mosq1Animation.update();
            mosq1Animation.render();
        } else {
            valentineTextOnly.update();
            valentineTextOnly.render();
        }

        if (frame % writeEvery == 0) {
            std::ostringstream name;
            name << "sim_frames/" << prefix << "_" << std::setfill('0') << std::setw(4) << frame << ".ppm";
            display.saveFramePPM(name.str());
        }

        delay(33);
    }

    std::cout << "Simulation complete for " << prefix << ". Frames written to ./sim_frames (PPM format)." << std::endl;
    std::cout << "Tip: open them with image/video tools or convert to GIF/MP4." << std::endl;
    return 0;
}
