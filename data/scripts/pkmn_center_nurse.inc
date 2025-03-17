EventScript_PkmnCenterNurse::
	goto_if_questlog EventScript_ReleaseEnd
	message Text_WelcomeWantToHealPkmn
	waitmessage
	multichoice 19, 8, MULTICHOICE_YES_NO, FALSE | (TRUE << 1)
	switch VAR_RESULT
	case 0, EventScript_PkmnCenterNurse_HealPkmn
	case 1 EventScript_PkmnCenterNurse_Goodbye
	case SCR_MENU_CANCEL, EventScript_PkmnCenterNurse_Goodbye
	end

EventScript_PkmnCenterNurse_HealPkmn::
	incrementgamestat GAME_STAT_USED_POKECENTER
	message Text_TakeYourPkmnForFewSeconds
	waitmessage
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn
	special SetUsedPkmnCenterQuestLogEvent
	goto EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom
	end

EventScript_PkmnCenterNurse_TakeAndHealPkmn::
	applymovement VAR_LAST_TALKED, Movement_PkmnCenterNurse_Turn @ Changed from Common_Movement_WalkInPlaceFasterLeft to force the follower to enter their Poké Ball
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	return

EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom::
	specialvar VAR_RESULT, IsPlayerNotInTrainerTowerLobby
	goto_if_eq VAR_RESULT, FALSE, EventScript_PkmnCenterNurse_ReturnPkmn
	specialvar VAR_RESULT, BufferUnionRoomPlayerName
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, 0, EventScript_PkmnCenterNurse_ReturnPkmn
	goto_if_eq VAR_0x8008, 1, EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom
	end

EventScript_PkmnCenterNurse_ReturnPkmn::
	message Text_RestoredPkmnToFullHealth
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain
	return

EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom::
	goto_if_set FLAG_SYS_INFORMED_OF_LOCAL_WIRELESS_PLAYER, EventScript_PkmnCenterNurse_ReturnPkmn
	msgbox Text_RestoredPkmnToFullHealth
	setflag FLAG_SYS_INFORMED_OF_LOCAL_WIRELESS_PLAYER
	message Text_PlayerIsPlayingRightNowGoForIt
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain
	return

EventScript_PkmnCenterNurse_Goodbye::
	msgbox Text_WeHopeToSeeYouAgain
	return

Movement_Bow::
	nurse_joy_bow
	delay_4
	step_end

Movement_PkmnCenterNurse_Turn:
	walk_in_place_faster_left
	step_end

NettuxHelper_EventScript_NameRater::
        lock
        faceplayer
        msgbox NettuxHelper_Text_WantMeToRateNicknames, MSGBOX_YESNO
        goto_if_eq VAR_RESULT, YES, NettuxHelper_EventScript_ChooseMon
        goto_if_eq VAR_RESULT, NO, NettuxHelper_EventScript_DontRateNickname
        end

NettuxHelper_EventScript_ChooseMon::
        msgbox NettuxHelper_Text_CritiqueWhichMonsNickname
        special ChoosePartyMon
        waitstate
        goto_if_lt VAR_0x8004, PARTY_SIZE, NettuxHelper_EventScript_CheckCanRateMon
        goto_if_ge VAR_0x8004, PARTY_SIZE, NettuxHelper_EventScript_DontRateNickname
        end

NettuxHelper_EventScript_DontRateNickname::
        msgbox NettuxHelper_Text_ISeeComeVisitAgain
        release
        end

NettuxHelper_EventScript_CheckCanRateMon::
        specialvar VAR_RESULT, GetPartyMonSpecies
        goto_if_eq VAR_RESULT, SPECIES_EGG, NettuxHelper_EventScript_CantNicknameEgg
        special BufferMonNickname
        special IsMonOTIDNotPlayers
        goto_if_eq VAR_RESULT, TRUE, NettuxHelper_EventScript_CantNicknameTradeMon
        specialvar VAR_RESULT, IsMonOTNameNotPlayers
        special BufferMonNickname
        goto_if_eq VAR_RESULT, TRUE, NettuxHelper_EventScript_CantNicknameTradeMon
        msgbox NettuxHelper_Text_GiveItANicerName, MSGBOX_YESNO
        goto_if_eq VAR_RESULT, YES, NettuxHelper_EventScript_ChooseNewNickname
        goto_if_eq VAR_RESULT, NO, NettuxHelper_EventScript_DontRateNickname
        end

NettuxHelper_EventScript_CantNicknameEgg::
        msgbox NettuxHelper_Text_ThatIsMerelyAnEgg
        release
        end

NettuxHelper_EventScript_CantNicknameTradeMon::
        msgbox NettuxHelper_Text_TrulyImpeccableName
        release
        end

NettuxHelper_EventScript_ChooseNewNickname::
        msgbox NettuxHelper_Text_WhatShallNewNicknameBe
        call EventScript_ChangePokemonNickname
        specialvar VAR_RESULT, NameRaterWasNicknameChanged
        special BufferMonNickname
        goto_if_eq VAR_RESULT, TRUE, NettuxHelper_EventScript_ChoseNewNickname
        msgbox NettuxHelper_Text_FromNowOnShallBeKnownAsSameName
        release
        end

NettuxHelper_EventScript_ChoseNewNickname::
        msgbox NettuxHelper_Text_FromNowOnShallBeKnownAsName
        release
        end

NettuxHelper_Text_WantMeToRateNicknames::
    .string "Hello, hello!\n"
    .string "I am the official NAME RATER!\p"
    .string "Want me to rate the nicknames of\n"
    .string "your POKéMON?$"

NettuxHelper_Text_CritiqueWhichMonsNickname::
    .string "Which POKéMON's nickname should\n"
    .string "I critique?$"

NettuxHelper_Text_GiveItANicerName::
    .string "{STR_VAR_1}, is it?\n"
    .string "That is a decent nickname!\p"
    .string "But, would you like me to give it\n"
    .string "a nicer name?\p"
    .string "How about it?$"

NettuxHelper_Text_WhatShallNewNicknameBe::
    .string "Ah, good. Then, what shall the new\n"
    .string "nickname be?$"

NettuxHelper_Text_FromNowOnShallBeKnownAsName::
    .string "Done! From now on, this POKéMON\n"
    .string "shall be known as {STR_VAR_1}!\p"
    .string "It is a better name than before!\n"
    .string "How fortunate for you!$"

NettuxHelper_Text_ISeeComeVisitAgain::
    .string "I see.\n"
    .string "Do come visit again.$"

NettuxHelper_Text_FromNowOnShallBeKnownAsSameName::
    .string "Done! From now on, this POKéMON\n"
    .string "shall be known as {STR_VAR_1}!\p"
    .string "It looks no different from before,\n"
    .string "and yet, this is vastly superior!\p"
    .string "How fortunate for you!$"

NettuxHelper_Text_TrulyImpeccableName::
    .string "{STR_VAR_1}, is it?\n"
    .string "That is a truly impeccable name!\p"
    .string "Take good care of {STR_VAR_1}!$"

NettuxHelper_Text_ThatIsMerelyAnEgg::
    .string "Now, now.\n"
    .string "That is merely an EGG!$"

NettuxHelper_EventScript_PokeHelper::
	lockall
	faceplayer
	message NettuxHelper_EventScript_PokeHelper_Text_0
	multichoice 25, 5, MULTICHOICE_NETTUX_POKE_HELPER, 0
	closemessage
	compare VAR_RESULT, 0
	goto_if_eq NettuxHelper_EventScript_PokeHelper_2
	compare VAR_RESULT, 1
	goto_if_eq NettuxHelper_EventScript_PokeHelper_3
	compare VAR_RESULT, 2
	goto_if_eq NettuxHelper_EventScript_PokeHelper_4
NettuxHelper_EventScript_PokeHelper_1:
	msgbox NettuxHelper_EventScript_PokeHelper_Text_1
	waitmessage
	releaseall
	return

NettuxHelper_EventScript_PokeHelper_2:
	call NettuxHelper_EventScript_NameRater
	goto NettuxHelper_EventScript_PokeHelper_1

NettuxHelper_EventScript_PokeHelper_3:
	call NettuxHelper_EventScript_Move_Relearner
	goto NettuxHelper_EventScript_PokeHelper_1

NettuxHelper_EventScript_PokeHelper_4:
	call NettuxHelper_EventScript_Hyper_Train
	goto NettuxHelper_EventScript_PokeHelper_1


NettuxHelper_EventScript_Hyper_Train::
	msgbox NettuxHelper_EventScript_Hyper_Train_Text_0
	call NettuxHelper_HyperTrain
	releaseall
	return


NettuxHelper_EventScript_Move_Relearner::
	msgbox NettuxHelper_EventScript_Move_Relearner_Text_0
	applymovement OBJ_EVENT_ID_FOLLOWER, EnterPokeballMovement
        waitmovement OBJ_EVENT_ID_FOLLOWER
	call NettuxHelper_EventScript_ChooseMonToTutor
	releaseall
	applymovement VAR_LAST_TALKED, Movement_PkmnCenterNurse_Turn @ Changed from Common_Movement_WalkInPlaceFasterLeft to force the follower to enter their Poké Ball
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	callnative UpdateFollowingPokemon
	return


NettuxHelper_EventScript_PokeHelper_Text_0:
	.string "Do you want a pokemon?$"

NettuxHelper_EventScript_PokeHelper_Text_1:
	.string "Come back anytime!$"

NettuxHelper_EventScript_Hyper_Train_Text_0:
	.string "Hyper Train!$"

NettuxHelper_EventScript_Move_Relearner_Text_0:
	.string "Move Relearner!$"




NettuxHelper_EventScript_MartHelper::
	lockall
	faceplayer
	message NettuxHelper_EventScript_MartHelper_Text_1
	multichoice 25, 5, MULTICHOICE_NETTUX_MART_HELPER, 0
	closemessage
	compare VAR_RESULT, 0
	goto_if_eq NettuxHelper_EventScript_MartHelper_5
	compare VAR_RESULT, 1
	goto_if_eq NettuxHelper_EventScript_MartHelper_6
	compare VAR_RESULT, 2
	goto_if_eq NettuxHelper_EventScript_MartHelper_7
	compare VAR_RESULT, 3
	goto_if_eq NettuxHelper_EventScript_MartHelper_8
NettuxHelper_EventScript_MartHelper_4:
	msgbox NettuxHelper_EventScript_MartHelper_Text_2
	waitmessage
	release
	return

NettuxHelper_EventScript_MartHelper_5:
	call NettuxHelper_EventScript_BattleMart
	goto NettuxHelper_EventScript_MartHelper_4

NettuxHelper_EventScript_MartHelper_6:
	call NettuxHelper_EventScript_TrainingMart
	goto NettuxHelper_EventScript_MartHelper_4

NettuxHelper_EventScript_MartHelper_7:
	call NettuxHelper_EventScript_TMMart
	goto NettuxHelper_EventScript_MartHelper_4

NettuxHelper_EventScript_MartHelper_8:
	call NettuxHelper_EventScript_EvolveMart
	goto NettuxHelper_EventScript_MartHelper_4


NettuxHelper_EventScript_BattleMart::
	lock
	message NettuxHelper_EventScript_BattleMart_Text_0
	waitmessage
	pokemart NettuxBattleMartItems
	msgbox NettuxHelper_EventScript_BattleMart_Text_1
	release
	return


NettuxHelper_EventScript_TrainingMart::
	lock
	message NettuxHelper_EventScript_TrainingMart_Text_0
	waitmessage
	pokemart NettuxTrainingMartItems
	msgbox NettuxHelper_EventScript_BattleMart_Text_1
	release
	return


NettuxHelper_EventScript_TMMart::
	lock
	message NettuxHelper_EventScript_TMMart_Text_0
	waitmessage
	pokemart NettuxTMMartItems
	msgbox NettuxHelper_EventScript_BattleMart_Text_1
	release
	return


NettuxHelper_EventScript_EvolveMart::
	lock
	message NettuxHelper_EventScript_EvolveMart_Text_0
	waitmessage
	pokemart NettuxEvolveMartItems
	msgbox NettuxHelper_EventScript_BattleMart_Text_1
	release
	return

	.align 2
NettuxBattleMartItems:
	.2byte ITEM_EXP_SHARE
	.2byte ITEM_NONE

	.align 2
NettuxTrainingMartItems:
	.2byte ITEM_EXP_CANDY_XS
	.2byte ITEM_NONE

	.align 2
NettuxTMMartItems:
	.2byte ITEM_TM_PROTECT
	.2byte ITEM_NONE

	.align 2
NettuxEvolveMartItems:
	.2byte ITEM_LINKING_CORD
	.2byte ITEM_NONE

NettuxHelper_EventScript_MartHelper_Text_1:
	.string "Do you want a pokemon?$"

NettuxHelper_EventScript_MartHelper_Text_2:
	.string "Come back anytime!$"

NettuxHelper_EventScript_BattleMart_Text_0:
	.string "Ready to Battle?$"

NettuxHelper_EventScript_BattleMart_Text_1:
	.string "Level up!$"

NettuxHelper_EventScript_TrainingMart_Text_0:
	.string "Ready to Train?$"

NettuxHelper_EventScript_TMMart_Text_0:
	.string "Ready to Learn?$"

NettuxHelper_EventScript_EvolveMart_Text_0:
	.string "Ready to Evolve?$"

NettuxHelper_EventScript_ChooseMonToTutor::
        msgbox NettuxHelper_Text_TutorWhichMon
        special ChooseMonForMoveRelearner
        waitstate
        goto_if_ge VAR_0x8004, PARTY_SIZE, NettuxHelper_EventScript_EndTutorMove
        special IsSelectedMonEgg
        goto_if_eq VAR_RESULT, TRUE, NettuxHelper_EventScript_CantTutorEgg
        goto_if_eq VAR_0x8005, 0, NettuxHelper_EventScript_NoMoveToTutorMon
        goto NettuxHelper_EventScript_ChooseMoveToTeach
        end

NettuxHelper_EventScript_ChooseMoveToTeach::
        msgbox NettuxHelper_Text_TeachWhichMove
        special TeachMoveRelearnerMove
        waitstate
        end

NettuxHelper_EventScript_NoMoveToTutorMon::
        msgbox NettuxHelper_Text_NoMoveICanTeachThatMon
        goto NettuxHelper_EventScript_ChooseMonToTutor
        end

NettuxHelper_EventScript_CantTutorEgg::
        msgbox NettuxHelper_Text_CantTeachAnEgg
        goto NettuxHelper_EventScript_ChooseMonToTutor
        end

NettuxHelper_EventScript_EndTutorMove::
        release
        end

NettuxHelper_Text_WantMeToTeachMove::
    .string "Do you want me to teach a move\n"
    .string "to a POKéMON of yours?$"

NettuxHelper_Text_TutorWhichMon::
    .string "Which POKéMON needs tutoring?$"

NettuxHelper_Text_TeachWhichMove::
    .string "Which move should I teach?$"

NettuxHelper_Text_NoMoveICanTeachThatMon::
    .string "Sorry…\p"
    .string "It doesn't appear as if I have a\n"
    .string "move I can teach that POKéMON.$"

NettuxHelper_Text_CantTeachAnEgg::
    .string "Hunh? There isn't a single move\n"
    .string "that I can teach an EGG.$"


NettuxHelper_SetIVs::
        checkmoney 5000  
	goto_if_eq VAR_RESULT, FALSE, NettuxHelper_EventScript_NotEnoughMoneyHyperTrain
        special ChoosePartyMon
        waitstate
        compare VAR_0x8004, PARTY_SIZE
        goto_if_ge NettuxHelper_ButIRefuse
        specialvar VAR_RESULT, GetPartyMonSpecies
        compare VAR_RESULT, SPECIES_EGG
        goto_if_eq NettuxHelper_Rejected
        goto NettuxHelper_SetIVs_Proceed

NettuxHelper_SetIVs_Proceed:
        lockall
        faceplayer
        removemoney 5000
        special MaxHpIvs
        special MaxAtkIvs
        special MaxDefIvs
        special MaxSpdIvs
        special MaxSpAtkIvs
        special MaxSpDefIvs
        msgbox NettuxHelper_IVsSetSuccessfully 4
        closemessage
        releaseall
        end

NettuxHelper_ButIRefuse:
        lockall
        faceplayer
        msgbox NettuxHelper_ComeBackAnytime 6
        releaseall
        end

NettuxHelper_Rejected:
        lockall
        faceplayer
        msgbox NettuxHelper_Reject 6
        releaseall
        end

NettuxHelper_IVsSetSuccessfully:
        .string "The IVs of your Pokémon's stats\n"
        .string "have been successfully set to 31.\p"
        .string "Come back anytime.$"

NettuxHelper_Reject:
        .string "Sorry, I can't modify the stats\n"
        .string "of an EGG.$"

NettuxHelper_ComeBackAnytime:
        .string "Come back anytime.$"

NettuxHelper_HyperTrain::
	faceplayer
	showmoneybox 0, 0
	msgbox NettuxHelper_HyperTrain_Text_0, MSGBOX_YESNO
	hidemoneybox
	compare VAR_RESULT, YES
	goto_if_eq NettuxHelper_HyperTrain_1
	msgbox NettuxHelper_HyperTrain_Text_1, MSGBOX_NPC
	applymovement VAR_LAST_TALKED, Movement_PkmnCenterNurse_Turn @ Changed from Common_Movement_WalkInPlaceFasterLeft to force the follower to enter their Poké Ball
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	callnative UpdateFollowingPokemon
	return

NettuxHelper_HyperTrain_1:
	goto NettuxHelper_SetIVs
	return

NettuxHelper_EventScript_NotEnoughMoneyHyperTrain::
	msgbox NettuxHelper_EventScript_NotEnoughMoneyHyperTrain_Text_0, MSGBOX_NPC
	return

NettuxHelper_HyperTrain_Text_0:
	.string "Hyper train a pokemon for ¥5000?$"

NettuxHelper_HyperTrain_Text_1:
	.string "Come back anytime.$"

NettuxHelper_EventScript_NotEnoughMoneyHyperTrain_Text_0:
	.string "Not enough money.$"
