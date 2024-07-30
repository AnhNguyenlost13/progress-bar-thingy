/**
 * Include the Geode headers.
 */
#include "Geode/modify/Modify.hpp"
#include "ccTypes.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
/*
#include <sys/_types/_int64_t.h> // fuck you
*/

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;


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
	void startGame() {
		PlayLayer::startGame();
		if ((!m_level->isPlatformer()) && m_progressFill){
			ccColor3B nya = paint();
			auto progress_bar = static_cast<CCSprite* >(this->getChildByID("progress-bar")->getChildren()->objectAtIndex(0)); // TODO: use the member instead
			progress_bar->setColor(nya);
		}
	}
};