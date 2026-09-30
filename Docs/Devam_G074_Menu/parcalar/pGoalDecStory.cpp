// ---------------------------------------------------------------------------------------------------------------
// Pages

TSharedRef<SWidget> SMarketMenu::GoalList()
{
    auto Row = [this](TFunction<bool()> Done, TFunction<FString()> Text) -> TSharedRef<SWidget>
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Dot(Done) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label(Text, 11, ERole::Text) ];
    };
    auto G = [this] { return Game.Get(); };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && !MarketCampaign::DebtOpen(G()->State); }, [] { return FString(TEXT("Baban\u0131n borcu kapans\u0131n")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.Cash >= MarketCampaign::ExpandCash; },
              [G] { return FString::Printf(TEXT("Kasada %s  (\u015fu an %s)"), *MarketMenuUi::Tl(MarketCampaign::ExpandCash), G() ? *MarketMenuUi::Tl(G()->State.Cash) : TEXT("")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.ProfitableDays >= MarketCampaign::ExpandProfitableDays; },
              [G] { return FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn  (%d / %d)"), MarketCampaign::ExpandProfitableDays, G() ? G()->State.ProfitableDays : 0, MarketCampaign::ExpandProfitableDays); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.MarketShare >= MarketCampaign::ExpandShare; },
              [G] { return FString::Printf(TEXT("Yerel pay en az %%%.0f  (\u015fu an %%%.0f)"), MarketCampaign::ExpandShare, G() ? G()->State.MarketShare : 0.f); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
        [
            Button([G] { return G() && G()->State.bSecondStore ? FString(TEXT("\u0130kinci \u015fube a\u00e7\u0131k")) : FString::Printf(TEXT("\u0130kinci \u015fubeyi a\u00e7  (%s)"), *MarketMenuUi::Tl(MarketCampaign::ExpandCash)); },
                [this] { Do(TEXT("Expand")); }, true,
                [G] { return G() && MarketCampaign::ExpandBlock(G()->State) == MarketCampaign::EExpandBlock::None; })
        ];
}

TSharedRef<SWidget> SMarketMenu::DecisionCard()
{
    // G-066: the first choice waiting for the player (story scene or neighbourhood event), with its options.
    auto G = [this] { return Game.Get(); };
    auto Pending = [G]() -> const FMarketDecision* { return G() ? MarketEvents::Pending(G()->State) : nullptr; };
    TSharedRef<SHorizontalBox> Options = SNew(SHorizontalBox);
    for (int32 Option = 0; Option < 3; ++Option)
    {
        Options->AddSlot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Pending, Option] { const FMarketDecision* D = Pending(); return D && D->Options.IsValidIndex(Option) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([Pending, Option] { const FMarketDecision* D = Pending(); return D && D->Options.IsValidIndex(Option) ? D->Options[Option] : FString(); },
                [this, Option] { Manage(TEXT("Decide"), Option); }, Option == 0) ]
        ];
    }
    return SNew(SBox).Visibility_Lambda([Pending] { return Pending() ? EVisibility::Visible : EVisibility::Collapsed; }).Padding(FMargin(0.f, 0.f, 0.f, 12.f))
    [
        Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Label([Pending] { const FMarketDecision* D = Pending(); return D ? D->Title.ToUpper() : FString(); }, 9, ERole::Accent, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)[ Label([Pending] { const FMarketDecision* D = Pending(); return D ? D->Text : FString(); }, 12, ERole::Text, false, true) ]
            + SVerticalBox::Slot().AutoHeight()[ Options ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Label([G, Pending]
            {
                const FMarketDecision* D = Pending();
                if (!D || !G()) return FString();
                const int32 More = G()->State.Decisions.Num() - 1;
                return FString::Printf(TEXT("Se\u00e7mezsen %d. g\u00fcn\u00fcn sonunda \"%s\" ge\u00e7erli olur.%s"), D->Deadline,
                    D->Options.IsValidIndex(D->DefaultOption) ? *D->Options[D->DefaultOption] : TEXT(""), More > 0 ? *FString::Printf(TEXT(" S\u0131rada %d karar daha var."), More) : TEXT(""));
            }, 9, ERole::Muted, false, true) ])
    ];
}

TSharedRef<SWidget> SMarketMenu::StoryCard()
{
    // G-066: the chapter, its goals and the shop's identity.
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Goals = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 5; ++Slot)
    {
        auto Goal = [G, Slot](MarketStory::FObjective& Out)
        {
            if (!G()) return false;
            const TArray<MarketStory::FObjective> All = MarketStory::Objectives(G()->State);
            if (!All.IsValidIndex(Slot)) return false;
            Out = All[Slot];
            return true;
        };
        Goals->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([Goal] { MarketStory::FObjective O; return Goal(O) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Dot([Goal] { MarketStory::FObjective O; return Goal(O) && O.bDone; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([Goal] { MarketStory::FObjective O; return Goal(O) ? O.Text + (O.bLater ? FString(TEXT(" (sonraki g\u00fcncelleme)")) : FString()) : FString(); }, 11, ERole::Text) ]
        ];
    }
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [ Label([G] { return G() ? FString::Printf(TEXT("B\u00d6L\u00dcM %d \u00b7 %s"), G()->State.Story.Chapter, *MarketStory::ChapterTitle(G()->State.Story.Chapter).ToUpper()) : FString(); }, 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ Goals ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketStoryState& Story = G()->State.Story;
            FString Text = FString::Printf(TEXT("Kimlik: %s"), *MarketStory::IdentityName(static_cast<MarketStory::EIdentity>(Story.Identity)));
            if (Story.Memories.Num() > 0) Text += TEXT(" \u00b7 son hat\u0131ra: ") + Story.Memories.Last();
            return Text;
        }, 10, ERole::Muted, false, true) ]);
}
