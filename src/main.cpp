/**
 * Include the Geode headers.
 */
#include "Geode/modify/Modify.hpp"
#include "ccTypes.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <sys/_types/_int64_t.h>

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

/**
 * `$modify` lets you extend and modify GD's classes.
 * To hook a function in Geode, simply $modify the class
 * and write a new function definition with the signature of
 * the function you want to hook.
 *
 * Here we use the overloaded `$modify` macro to set our own class name,
 * so that we can use it for button callbacks.
 *
 * Notice the header being included, you *must* include the header for
 * the class you are modifying, or you will get a compile error.
 *
 * Another way you could do this is like this:
 *
 * struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {};
 */
#include <Geode/modify/PlayLayer.hpp>

ccColor3B paint(){
	ccColor3B uwu;

	/**
	* Options:
	* - Default (vanilla behavior): P1 color
	* - Case 1: P2 color
	* - Case 2: Player glow color
	* - Case 3: Manual (configure in settings)
	*/
	switch(Mod::get()->getSettingValue<int64_t>("mode")){
		case 1: 
			uwu = GameManager::sharedState()->colorForIdx(GameManager::sharedState()->getPlayerColor2());
			break;
		case 2:
			uwu = GameManager::sharedState()->colorForIdx(GameManager::sharedState()->getPlayerGlowColor());
			break;
		case 3:
			uwu = Mod::get()->getSettingValue<ccColor3B>("color");
			break;
		default:
			uwu = GameManager::sharedState()->colorForIdx(GameManager::sharedState()->getPlayerColor());
			break;
	}
	return uwu;
}

class $modify(PlayLayer) {
	bool init(GJGameLevel* level, bool a, bool b) {
		if (!PlayLayer::init(level, a, b)) {
			return false;
		}
		if (!PlayLayer::get()->m_level->isPlatformer()){
			ccColor3B cb = paint();
			auto progress_bar = static_cast<CCSprite* >(this->getChildByID("progress-bar")->getChildren()->objectAtIndex(0));
			progress_bar->setColor(cb);
		}
		return true;
	}
};