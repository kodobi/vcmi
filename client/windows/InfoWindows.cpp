/*
 * InfoWindows.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "InfoWindows.h"

#include "../CGameInfo.h"
#include "../PlayerLocalState.h"
#include "../CPlayerInterface.h"
#include "../CMusicHandler.h"

#include "../adventureMap/AdventureMapInterface.h"
#include "../battle/BattleInterface.h"
#include "../battle/BattleInterfaceClasses.h"
#include "../gui/CGuiHandler.h"
#include "../gui/CursorHandler.h"
#include "../gui/Shortcut.h"
#include "../gui/WindowHandler.h"
#include "../render/Canvas.h"
#include "../renderSDL/SDL_Extensions.h"
#include "../widgets/Buttons.h"
#include "../widgets/CComponent.h"
#include "../widgets/Images.h"
#include "../widgets/MiscWidgets.h"
#include "../widgets/TextControls.h"
#include "../windows/CMessage.h"

#include "../../CCallback.h"

#include "../../lib/CConfigHandler.h"
#include "../../lib/CondSh.h"
#include "../../lib/CGeneralTextHandler.h" //for Unicode related stuff
#include "../../lib/mapObjects/CGCreature.h"
#include "../../lib/mapObjects/CGHeroInstance.h"
#include "../../lib/mapObjects/CGTownInstance.h"
#include "../../lib/mapObjects/MiscObjects.h"
#include "../../lib/gameState/InfoAboutArmy.h"

#include <SDL_surface.h>

void CSelWindow::selectionChange(unsigned to)
{
	for (unsigned i=0;i<components.size();i++)
	{
		auto pom = std::dynamic_pointer_cast<CSelectableComponent>(components[i]);
		if (!pom)
			continue;
		pom->select(i==to);
	}
	redraw();
}

CSelWindow::CSelWindow(const std::string &Text, PlayerColor player, int charperline, const std::vector<std::shared_ptr<CSelectableComponent>> & comps, const std::vector<std::pair<AnimationPath, CFunctionList<void()> > > &Buttons, QueryID askID)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);

	backgroundTexture = std::make_shared<CFilledTexture>(ImagePath::builtin("DiBoxBck"), pos);

	ID = askID;
	for (int i = 0; i < Buttons.size(); i++)
	{
		buttons.push_back(std::make_shared<CButton>(Point(0, 0), Buttons[i].first, CButton::tooltip(), Buttons[i].second));
		if (!i  &&  askID.getNum() >= 0)
			buttons.back()->addCallback(std::bind(&CSelWindow::madeChoice, this));
		buttons[i]->addCallback(std::bind(&CInfoWindow::close, this)); //each button will close the window apart from call-defined actions
	}

	text = std::make_shared<CTextBox>(Text, Rect(0, 0, 250, 100), 0, FONT_MEDIUM, ETextAlignment::CENTER, Colors::WHITE);

	if (buttons.size() > 1 && askID.getNum() >= 0) //cancel button functionality
	{
		buttons.back()->addCallback([askID]() {
			LOCPLINT->cb.get()->selectionMade(0, askID);
		});
		//buttons.back()->addCallback(std::bind(&CCallback::selectionMade, LOCPLINT->cb.get(), 0, askID));
	}

	if(buttons.size() == 1)
		buttons.front()->assignedKey = EShortcut::GLOBAL_RETURN;

	if(buttons.size() == 2)
	{
		buttons.front()->assignedKey = EShortcut::GLOBAL_ACCEPT;
		buttons.back()->assignedKey = EShortcut::GLOBAL_CANCEL;
	}

	for(int i=0;i<comps.size();i++)
	{
		comps[i]->recActions = 255-DISPOSE;
		addChild(comps[i].get());
		components.push_back(comps[i]);
		comps[i]->onSelect = std::bind(&CSelWindow::selectionChange,this,i);
		comps[i]->onChoose = std::bind(&CSelWindow::madeChoiceAndClose,this);
		if(i<8)
			comps[i]->assignedKey = vstd::next(EShortcut::SELECT_INDEX_1,i);
	}
	CMessage::drawIWindow(this, Text, player);
}

void CSelWindow::madeChoice()
{
	if(ID.getNum() < 0)
		return;
	int ret = -1;
	for (int i=0;i<components.size();i++)
	{
		if(std::dynamic_pointer_cast<CSelectableComponent>(components[i])->selected)
		{
			ret = i;
		}
	}
	LOCPLINT->cb->selectionMade(ret+1,ID);
}

void CSelWindow::madeChoiceAndClose()
{
	madeChoice();
	close();
}

CInfoWindow::CInfoWindow(std::string Text, PlayerColor player, const TCompsInfo & comps, const TButtonsInfo & Buttons)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);

	backgroundTexture = std::make_shared<CFilledTexture>(ImagePath::builtin("DiBoxBck"), pos);

	ID = QueryID(-1);
	for(auto & Button : Buttons)
	{
		std::shared_ptr<CButton> button = std::make_shared<CButton>(Point(0,0), Button.first, CButton::tooltip(), std::bind(&CInfoWindow::close, this));
		button->setBorderColor(Colors::METALLIC_GOLD);
		button->addCallback(Button.second); //each button will close the window apart from call-defined actions
		buttons.push_back(button);
	}

	text = std::make_shared<CTextBox>(Text, Rect(0, 0, 250, 100), 0, FONT_MEDIUM, ETextAlignment::CENTER, Colors::WHITE);
	if(!text->slider)
	{
		int finalWidth = std::min(250, text->label->textSize.x + 32);
		int finalHeight = text->label->textSize.y;
		text->resize(Point(finalWidth, finalHeight));
	}

	if(buttons.size() == 1)
		buttons.front()->assignedKey = EShortcut::GLOBAL_RETURN;

	if(buttons.size() == 2)
	{
		buttons.front()->assignedKey = EShortcut::GLOBAL_ACCEPT;
		buttons.back()->assignedKey = EShortcut::GLOBAL_CANCEL;
	}

	for(auto & comp : comps)
	{
		comp->recActions = 0xff & ~DISPOSE;
		addChild(comp.get());
		comp->recActions &= ~(SHOWALL | UPDATE);
		components.push_back(comp);
	}

	CMessage::drawIWindow(this,Text,player);
}

CInfoWindow::CInfoWindow()
{
	ID = QueryID(-1);
}

void CInfoWindow::close()
{
	WindowBase::close();

	if(LOCPLINT)
		LOCPLINT->showingDialog->setn(false);
}

void CInfoWindow::showAll(Canvas & to)
{
	CIntObject::showAll(to);
	CMessage::drawBorder(LOCPLINT ? LOCPLINT->playerID : PlayerColor(1), to.getInternalSurface(), pos.w, pos.h, pos.x, pos.y);
}

CInfoWindow::~CInfoWindow() = default;

void CInfoWindow::showInfoDialog(const std::string &text, const TCompsInfo & components, PlayerColor player)
{
	GH.windows().pushWindow(CInfoWindow::create(text, player, components));
}

void CInfoWindow::showYesNoDialog(const std::string & text, const TCompsInfo & components, const CFunctionList<void( ) > &onYes, const CFunctionList<void()> &onNo, PlayerColor player)
{
	assert(!LOCPLINT || LOCPLINT->showingDialog->get());
	std::vector<std::pair<AnimationPath,CFunctionList<void()> > > pom;
	pom.push_back( { AnimationPath::builtin("IOKAY.DEF"), 0 });
	pom.push_back( { AnimationPath::builtin("ICANCEL.DEF"), 0 });
	std::shared_ptr<CInfoWindow> temp =  std::make_shared<CInfoWindow>(text, player, components, pom);

	temp->buttons[0]->addCallback( onYes );
	temp->buttons[1]->addCallback( onNo );

	GH.windows().pushWindow(temp);
}

std::shared_ptr<CInfoWindow> CInfoWindow::create(const std::string &text, PlayerColor playerID, const TCompsInfo & components)
{
	std::vector<std::pair<AnimationPath,CFunctionList<void()> > > pom;
	pom.push_back({AnimationPath::builtin("IOKAY.DEF"), 0});
	return std::make_shared<CInfoWindow>(text, playerID, components, pom);
}

std::string CInfoWindow::genText(std::string title, std::string description)
{
	return std::string("{") + title + "}" + "\n\n" + description;
}

bool CRClickPopup::isPopupWindow() const
{
	return true;
}

void CRClickPopup::close()
{
	WindowBase::close();
}

void CRClickPopup::createAndPush(const std::string &txt, const CInfoWindow::TCompsInfo &comps)
{
	PlayerColor player = LOCPLINT ? LOCPLINT->playerID : PlayerColor(1); //if no player, then use blue
	if(settings["session"]["spectate"].Bool())//TODO: there must be better way to implement this
		player = PlayerColor(1);

	auto temp = std::make_shared<CInfoWindow>(txt, player, comps);
	temp->center(GH.getCursorPosition()); //center on mouse
#ifdef VCMI_MOBILE
	temp->moveBy({0, -temp->pos.h / 2});
#endif
	temp->fitToScreen(10);

	GH.windows().createAndPushWindow<CRClickPopupInt>(temp);
}

void CRClickPopup::createAndPush(const std::string & txt, std::shared_ptr<CComponent> component)
{
	CInfoWindow::TCompsInfo intComps;
	intComps.push_back(component);

	createAndPush(txt, intComps);
}

void CRClickPopup::createAndPush(const CGObjectInstance * obj, const Point & p, ETextAlignment alignment)
{
	auto iWin = createCustomInfoWindow(p, obj); //try get custom infowindow for this obj
	if(iWin)
	{
		GH.windows().pushWindow(iWin);
	}
	else
	{
		std::vector<Component> components;
		if (settings["general"]["enableUiEnhancements"].Bool())
		{
			if(LOCPLINT->localState->getCurrentHero())
				components = obj->getPopupComponents(LOCPLINT->localState->getCurrentHero());
			else
				components = obj->getPopupComponents(LOCPLINT->playerID);
		}

		std::vector<std::shared_ptr<CComponent>> guiComponents;
		for (auto & component : components)
			guiComponents.push_back(std::make_shared<CComponent>(component));

		if(LOCPLINT->localState->getCurrentHero())
			CRClickPopup::createAndPush(obj->getPopupText(LOCPLINT->localState->getCurrentHero()), guiComponents);
		else
			CRClickPopup::createAndPush(obj->getPopupText(LOCPLINT->playerID), guiComponents);
	}
}

CRClickPopupInt::CRClickPopupInt(std::shared_ptr<CIntObject> our)
{
	CCS->curh->hide();
	defActions = SHOWALL | UPDATE;
	our->recActions = defActions;
	inner = our;
	addChild(our.get(), false);
}

CRClickPopupInt::~CRClickPopupInt()
{
	CCS->curh->show();
}

Point CInfoBoxPopup::toScreen(Point p)
{
	auto bounds = adventureInt->terrainAreaPixels();

	vstd::abetween(p.x, bounds.top() + 100, bounds.bottom() - 100);
	vstd::abetween(p.y, bounds.left() + 100, bounds.right() - 100);

	return p;
}

CInfoBoxPopup::CInfoBoxPopup(Point position, const CGTownInstance * town)
	: CWindowObject(RCLICK_POPUP | PLAYER_COLORED, ImagePath::builtin("TOWNQVBK"), toScreen(position))
{
	InfoAboutTown iah;
	LOCPLINT->cb->getTownInfo(town, iah, LOCPLINT->localState->getCurrentTown()); //todo: should this be nearest hero?

	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	tooltip = std::make_shared<CTownTooltip>(Point(9, 10), iah);
}

CInfoBoxPopup::CInfoBoxPopup(Point position, const CGHeroInstance * hero)
	: CWindowObject(RCLICK_POPUP | PLAYER_COLORED, ImagePath::builtin("HEROQVBK"), toScreen(position))
{
	InfoAboutHero iah;
	LOCPLINT->cb->getHeroInfo(hero, iah, LOCPLINT->localState->getCurrentHero());//todo: should this be nearest hero?

	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	tooltip = std::make_shared<CHeroTooltip>(Point(9, 10), iah);
}

CInfoBoxPopup::CInfoBoxPopup(Point position, const CGGarrison * garr)
	: CWindowObject(RCLICK_POPUP | PLAYER_COLORED, ImagePath::builtin("TOWNQVBK"), toScreen(position))
{
	InfoAboutTown iah;
	LOCPLINT->cb->getTownInfo(garr, iah);

	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	tooltip = std::make_shared<CArmyTooltip>(Point(9, 10), iah);
}

CInfoBoxPopup::CInfoBoxPopup(Point position, const CGCreature * creature)
		: CWindowObject(RCLICK_POPUP | BORDERED, ImagePath::builtin("DIBOXBCK"), toScreen(position))
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	tooltip = std::make_shared<CreatureTooltip>(Point(9, 10), creature);
}

std::shared_ptr<WindowBase> CRClickPopup::createCustomInfoWindow(Point position, const CGObjectInstance * specific) //specific=0 => draws info about selected town/hero
{
	if(nullptr == specific)
		specific = LOCPLINT->localState->getCurrentArmy();

	if(nullptr == specific)
	{
		logGlobal->error("createCustomInfoWindow: no object to describe");
		return nullptr;
	}

	switch(specific->ID)
	{
	case Obj::HERO:
		return std::make_shared<CInfoBoxPopup>(position, dynamic_cast<const CGHeroInstance *>(specific));
	case Obj::TOWN:
		return std::make_shared<CInfoBoxPopup>(position, dynamic_cast<const CGTownInstance *>(specific));
	case Obj::MONSTER:
		return std::make_shared<CInfoBoxPopup>(position, dynamic_cast<const CGCreature *>(specific));
	case Obj::GARRISON:
	case Obj::GARRISON2:
		return std::make_shared<CInfoBoxPopup>(position, dynamic_cast<const CGGarrison *>(specific));
	default:
		return std::shared_ptr<WindowBase>();
	}
}
