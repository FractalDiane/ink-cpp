VAR bedroomLightState = (off, on_desk)

VAR knifeState = (under_bed)


//
// Knowledge chains
//


LIST BedKnowledge = neatly_made, crumpled_duvet, hastily_remade, body_on_bed, murdered_in_bed, murdered_while_asleep

LIST KnifeKnowledge = prints_on_knife, joe_seen_prints_on_knife,joe_wants_better_prints, joe_got_better_prints

LIST WindowKnowledge = steam_on_glass, fingerprints_on_glass, fingerprints_on_glass_match_knife

LIST OffOn = off, on
LIST SeenUnseen = unseen, seen

LIST GlassState = (none), steamed, steam_gone
LIST BedState = (made_up), covers_shifted, covers_off, bloodstain_visible

LIST Supporters = on_desk, on_floor, on_bed, under_bed, held, with_joe

LIST Inventory = (none), cane, knife

VAR knowledgeState = ()

-> main

=== function between(x, y) 
   ~ return knowledgeState? x && not (knowledgeState ^ y)

=== main
{ between ((fingerprints_on_glass, prints_on_knife),     fingerprints_on_glass_match_knife) }
{(Inventory ? cane)}
-> END
