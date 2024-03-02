/*
 * CAltarCreatures.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CMarketBase.h"

class CAltarCreatures : public CExperienceAltar, public CCreaturesSelling, public CMarketSlider
{
public:
	CAltarCreatures(const IMarket * market, const CGHeroInstance * hero);
	void update() override;
	void deselect() override;
	TExpType calcExpAltarForHero() override;
	void makeDeal() override;
	void sacrificeAll() override;

private:
	std::vector<int> unitsOnAltar;
	std::vector<int> expPerUnit;

	CMarketBase::SelectionParams getSelectionParams() const override;
	void updateAltarSlot(const std::shared_ptr<CTradeableItem> & slot);
	void readExpValues();
	void highlightingChanged();
	void onOfferSliderMoved(int newVal) override;
	void onSlotClickPressed(const std::shared_ptr<CTradeableItem> & newSlot, std::shared_ptr<CTradeableItem> & hCurSlot) override;
};
