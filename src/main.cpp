/**
 * Find and Replace
 * by DarthStorm
 * 
 * Look into FindAndReplacePopup::perform to change the find and replace behaviour
 * (and also the documentation for this mod)
 * since most of the code is just ui stuff
 * 
 * oh yh btw
 * im kinda new to c++ and geode (i use python)
 * so a lot of the patterns were taken from mat.circle-tool
 * simply because it's a very simple mod that does a simple thing in the editor
 * so ty lol
 * also to any learners i commented all of my code — even the most trivial tasks —
 * because ill probably forget most of this
 * TODO: delete this part when i get better at geode
 */

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>

#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>


using namespace geode::prelude;


// wrapper for FLAlertLayer::create, specialized for debugging
void alert(const gd::string& message = "", const char* title = "", const char* accept_message = "ok", float width = 300.0f) {
		FLAlertLayer::create(nullptr, title, message, accept_message, nullptr, 300.f)->show();

}

class FindAndReplacePopup : public Popup {
public:

	static int m_target_id;
	static int m_replace_id;

    static FindAndReplacePopup* create() {
		auto* node = new FindAndReplacePopup();
		if (node->init()) {
			node->autorelease();
			return node;
		} else {
			delete node;
			return nullptr;
		}
	}

	bool init() override {
		if (!Popup::init(300, 150)) return false;


		auto* layer = m_mainLayer;
		auto* menu = m_buttonMenu;
		
		this->setTitle("Find and Replace");



		float button_width = 68;

		// add target_id input
		auto target_id_input = geode::TextInput::create(60.f, ""); // create
		target_id_input->setCommonFilter(CommonFilter::Int);  // only allow int
		// set input -> m_target_id
		target_id_input->setString(fmt::to_string(m_target_id)); 
		target_id_input->setCallback([this](std::string const& str) {
			m_target_id = geode::utils::numFromString<float>(str).unwrapOr(m_target_id);
		});

		// position left center
		layer->addChildAtPosition(target_id_input, Anchor::Center, ccp(-60, 0));
		// add accompanying label
		layer->addChildAtPosition(CCLabelBMFont::create("Replace", "goldFont.fnt"), Anchor::Center, ccp(-60, 30));

		// add replace_id input
		auto replace_id_input = geode::TextInput::create(60.f, ""); // create
		replace_id_input->setCommonFilter(CommonFilter::Int); // ony allow int
		//set input -> m_replace_id
		replace_id_input->setString(fmt::to_string(m_replace_id));
		replace_id_input->setCallback([this](std::string const& str) {
			m_replace_id = geode::utils::numFromString<float>(str).unwrapOr(m_replace_id);
		});

		// position right center
		layer->addChildAtPosition(replace_id_input, Anchor::Center, ccp(60, 0));
		// add accompanying label
		layer->addChildAtPosition(CCLabelBMFont::create("With", "goldFont.fnt"), Anchor::Center, ccp(60, 30));


		// add confirm button (replace)
		menu->addChildAtPosition(
            CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Replace", button_width, true, "goldFont.fnt", "GJ_button_01.png", 0, 0.75f),
				this, menu_selector(FindAndReplacePopup::on_apply) // on_apply calls replace
			),
			// position bottom right
			Anchor::Center, ccp(60, -30)
		);
        
		// add exit button
		menu->addChildAtPosition(
			CCMenuItemSpriteExtra::create(
				ButtonSprite::create("Cancel", button_width, true, "goldFont.fnt", "GJ_button_01.png", 0, 0.75f),
				this, menu_selector(FindAndReplacePopup::onClose)
			),
			// position bottom left
			Anchor::Center, ccp(-60, -30)
		);

		// add info button
		auto info_btn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(FindAndReplacePopup::on_info)
		);
		// position top right relative to menu, like all info buttons in game
		menu->addChildAtPosition(info_btn, Anchor::TopRight, ccp(-19, -19));

		return true;
	}


	void on_apply(CCObject* thing) { // actually dont know what `thing` is
		auto* editor = GameManager::sharedState()->getEditorLayer()->m_editorUI;
		auto objs = editor->getSelectedObjects();
        perform(); // the function that does the thing
		onClose(thing); // TODO: make this not use thing
	}

	/**
	 * Function that actually does the finding and replacing.
	 * Grabs selected objects and replaces them with new objects with different IDs.
	 * Currently keeps:
	 * - position
	 * - rotation
	 * - scale
	 * - group IDs
	 * 
	 * TODO: replace more things
	 */
    void perform() {
		// function that actually does the finding and replacing
        log::info("replacing all selected {} with {}", m_target_id, m_replace_id);
		
		// sharedState is a singleton instance of GameManager
		// gets editor layer
        auto* editor = GameManager::sharedState()->getEditorLayer();
		// gets the editor ui from the editor layer
        auto* editor_ui = editor->m_editorUI;

		// get selected objects, we will be using them to spawn new objects
		auto* selected_objects = editor_ui->getSelectedObjects();

		// note: GameObject->m_objectID is the actual id of the object, ex. default block returns 1, yellow orb returns 36
		gd::string err_msg = "";
		// loop through selected objects, 
		for (auto* obj : CCArrayExt<GameObject*>(selected_objects)) {
			// obj is somehow a null pointer...
			if (!obj) {
				alert("object is a null pointer\nreport this to the devs\nas well as what you were doing before it happened", "oh nyoo");
				continue;
			}
			// if the object has an id we want to replace...
			if (obj->m_objectID == m_target_id) {

				// replace with another object with the replace_id (note that position is set here)
				auto* new_obj = editor->createObject(m_replace_id, obj->getPosition(), false);
				// i forgot why i did this instead of adding the check before the replacing even starts
				// guess it might protect against some more random stuff
				// - darth
				if (!new_obj) {
					err_msg = "Object ID is invalid. Object ID: " + std::to_string(m_replace_id);
					continue;
				}

				// set rotation, scaling, and groups
				new_obj->setRotation(obj->getRotation());
				new_obj->setScale(obj->getScale());
				new_obj->copyGroups(obj);

				// ok i know they said to not edit the list while looping through it but...
				editor_ui->deleteObject(obj, false); // noUndo = false -> can undo

			}
		}
		
		// alert the player if something went wrong
		if (err_msg != "") {
			FLAlertLayer::create(nullptr, "error",err_msg,"ok", nullptr, 300.f)->show();
		}
    }

	// help screen
	void on_info(CCObject*) {
		FLAlertLayer::create(nullptr, "Help",
			"Replace objects that have id <cg>Replace</c> with new objects that have id <cy>With</c>.\n",
			"ok", nullptr, 300.f
		)->show();
	}

};

int FindAndReplacePopup::m_target_id = 1;
int FindAndReplacePopup::m_replace_id = 2;

class $modify(MyEditorUI, EditorUI) {

	bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) {
            return false;
		}

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "keybind-find-and-replace"),
            [this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
                if (down && !repeat) {
					// TODO: when i figure out why i have tp
                    this->on_find_and_replace(nullptr);
                }
            }
        );

		return true;
	}

	void on_find_and_replace(CCObject*) {
		// if selected objects found is truthy ( != 0 )
		if (this->getSelectedObjects()->count()) {
			FindAndReplacePopup::create()->show();
		} else {
			FLAlertLayer::create("Info", "Select some objects to use <cf>Find and Replace</c>", "OK")->show();
		}
	}

	void createMoveMenu() {
		EditorUI::createMoveMenu();
		auto* btn = this->getSpriteButton("button.png"_spr, menu_selector(MyEditorUI::on_find_and_replace), nullptr, 0.9f);
		m_editButtonBar->m_buttonArray->addObject(btn);
		// magic taken straight from circle tool
		auto rows = GameManager::sharedState()->getIntGameVariable("0049");
		auto cols = GameManager::sharedState()->getIntGameVariable("0050");
		m_editButtonBar->reloadItems(rows, cols);
	}
};

