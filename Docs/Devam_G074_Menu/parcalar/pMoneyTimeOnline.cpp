TSharedRef<SWidget> SMarketMenu::MoneyCard()
{
    // G-067: bank, credit book and the last-day policy of perishable goods.
    auto G = [this] { return Game.Get(); };
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("PARA \u00b7 VERES\u0130YE \u00b7 TAZEL\u0130K"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketState& S = G()->State;
            return MarketFinance::Summary(S) + FString::Printf(TEXT("\nVeresiye limiti %s \u00b7 son kullanma: %s \u00b7 d\u00fcnk\u00fc fire %d adet"),
                S.CreditLimit > 0 ? *MarketMenuUi::Tl(S.CreditLimit) : TEXT("yok"), *MarketFreshness::PolicyName(static_cast<MarketFreshness::EPolicy>(S.FreshPolicy)), S.LastWasteUnits);
        }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Kredi 500 TL")); }, [this] { Manage(TEXT("TakeLoan"), 0); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Kredi 1.000 TL")); }, [this] { Manage(TEXT("TakeLoan"), 1); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Krediyi kapat")); }, [this] { Manage(TEXT("RepayLoan"), 0); }, false, [G] { return G() && G()->State.Loans.Num() > 0; }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Veresiye yok")); }, [this] { Manage(TEXT("CreditLimit"), 0); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Veresiye 20 TL")); }, [this] { Manage(TEXT("CreditLimit"), 1); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Veresiye 50 TL")); }, [this] { Manage(TEXT("CreditLimit"), 2); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Bor\u00e7lar\u0131 iste")); }, [this] { Manage(TEXT("CollectCredit"), 0); }, false, [G] { return G() && G()->State.Credit.Num() > 0; }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Son g\u00fcn: indirim")); }, [this] { Manage(TEXT("FreshPolicy"), 1); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Son g\u00fcn: ba\u011f\u0131\u015f")); }, [this] { Manage(TEXT("FreshPolicy"), 2); }) ]
        ]);
}

TSharedRef<SWidget> SMarketMenu::TimeCard()
{
    // G-071: play days without walking the shop (stops when a decision waits or the week ends), difficulty.
    auto G = [this] { return Game.Get(); };
    auto Closed = [G] { return G() && !G()->bOpen; };
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("ZAMAN \u00b7 ZORLUK"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ Label([G] { return G() ? FString::Printf(TEXT("Zorluk: %s \u00b7 ilerletilen g\u00fcn %d. \u0130lerletirken aile d\u00fckk\u00e2n\u0131 i\u015fletir: raflar\u0131 doldurur, \u00f6neri kadar sipari\u015f verir; karar gerekince durur."),
            *MarketSimulation::DifficultyName(static_cast<MarketSimulation::EDifficulty>(G()->State.Difficulty)), G()->State.AdvancedDays) : FString(); }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("1 g\u00fcn ilerlet")); }, [this] { Manage(TEXT("Advance"), 1); }, false, Closed) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("1 hafta ilerlet")); }, [this] { Manage(TEXT("Advance"), 7); }, false, Closed) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Rahat")); }, [this] { Manage(TEXT("Difficulty"), 0); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Normal")); }, [this] { Manage(TEXT("Difficulty"), 1); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Zor")); }, [this] { Manage(TEXT("Difficulty"), 2); }) ]
        ]);
}

TSharedRef<SWidget> SMarketMenu::OnlineCard()
{
    // G-069: order channels of the era, couriers, the missing-item rule and payment methods.
    auto G = [this] { return Game.Get(); };
    auto Era = [G](MarketOnline::EChannel Channel) { return [G, Channel] { return G() && G()->State.Day >= MarketOnline::OpenDay(Channel); }; };
    auto Toggle = [G](MarketOnline::EChannel Channel) { return [G, Channel] { return FString::Printf(TEXT("%s: %s"), *MarketOnline::ChannelName(Channel), G() && MarketOnline::IsOn(G()->State, Channel) ? TEXT("a\u00e7\u0131k") : TEXT("kapal\u0131")); }; };
    auto Flip = [this, G](MarketOnline::EChannel Channel) { return [this, G, Channel] { Manage(TEXT("OnlineChannel"), static_cast<int32>(Channel) * 10 + (G() && MarketOnline::IsOn(G()->State, Channel) ? 0 : 1)); }; };
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("S\u0130PAR\u0130\u015e \u00b7 KURYE \u00b7 \u00d6DEME"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ Label([G] { return G() ? MarketOnline::Summary(G()->State) + TEXT("\n") + MarketPayments::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Button(Toggle(MarketOnline::EChannel::Phone), Flip(MarketOnline::EChannel::Phone)) ]
            + SWrapBox::Slot()[ Button(Toggle(MarketOnline::EChannel::Web), Flip(MarketOnline::EChannel::Web), false, Era(MarketOnline::EChannel::Web)) ]
            + SWrapBox::Slot()[ Button(Toggle(MarketOnline::EChannel::Platform), Flip(MarketOnline::EChannel::Platform), false, Era(MarketOnline::EChannel::Platform)) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Kurye al")); }, [this] { Manage(TEXT("HireCourier"), 0); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Kurye b\u0131rak")); }, [this] { Manage(TEXT("FireCourier"), 0); }, false, [G] { return G() && G()->State.Online.Couriers > 0; }) ]
            + SWrapBox::Slot()[ Button([G] { return FString(G() && G()->State.Online.bFreeDelivery ? TEXT("Teslimat: \u00fccretsiz") : TEXT("Teslimat: \u00fccretli")); },
                                       [this, G] { Manage(TEXT("FreeDelivery"), G() && G()->State.Online.bFreeDelivery ? 0 : 1); }) ]
            + SWrapBox::Slot()[ Button([G] { static const TCHAR* Rules[3] = { TEXT("Eksikte: sor"), TEXT("Eksikte: benzeri"), TEXT("Eksikte: \u00e7\u0131kar") };
                                             return FString(Rules[G() ? FMath::Clamp<int32>(G()->State.Online.Substitute, 0, 2) : 1]); },
                                       [this, G] { Manage(TEXT("Substitute"), G() ? (G()->State.Online.Substitute + 1) % 3 : 1); }) ]
            + SWrapBox::Slot()[ Button([G] { return FString(G() && G()->State.Payments.bCard ? TEXT("POS: var") : TEXT("POS: yok")); },
                                       [this, G] { Manage(TEXT("Card"), G() && G()->State.Payments.bCard ? 0 : 1); }) ]
            + SWrapBox::Slot()[ Button([G] { return FString(G() && G()->State.Payments.bMealCard ? TEXT("Yemek kart\u0131: al\u0131n\u0131yor") : TEXT("Yemek kart\u0131: yok")); },
                                       [this, G] { Manage(TEXT("MealCard"), G() && G()->State.Payments.bMealCard ? 0 : 1); }) ]
        ]);
}
