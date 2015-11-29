# CacheBot README.md

## Outstanding conceptual problems
### Soft boundaries between price windows
### Rolling average spikes (possible solution: amortizing _bundles_ of transactions)
The problem: Our rolling average definition of the likely date of the next expense within a priceWindow or habit (for now, let's just call both a habit) can result in surges in the daily cost of that habit, when there is a flurry of expenses within that range in close succession. This is a problem because: The benefit of amortizing purchases is that it makes your budget _steerable_: that it adjusts on the margin, and allows you to make decisions on the margin. If your daily costs spike well above your income, it can be easy to lose a sense of insight or meaningfulness of the number. Just like with your account balances, if it zigzags all over the place in short time scales, it's no longer an intelligible metric for your financial health.

I don't claim that I know what is the right UX, but what I've been doing manually for many years: When I have a flurry of similar expenses within a couple of days, I tend to amortize them all over the same period (say, 1 month), and so each's contribution to my daily costs is linear. Essentially, this is like amortizing the bundle as a single thing. So a possible solution is to _amortize_ the bundle, as a single thing.

CacheBot would then be looking for these spikes of expenses as one kind of "habit", and would amortize a spike of expenses over a period of time, such as the time between these kinds of spikes. Just a thought.

## Determine fraction of an anticipated expense that has been saved for
### Yes/no buying decisions
#### Describe here how cache-bot currently determines $--/day total costs for price windows.
- in a bundle, there are transactions, at given dates. What I consider is the space between transactions,
ex: daysToNext = [5d, 2d, 10d, 0d, 2d, 5d]
and then there is the time from the last transaction to present: daysToPres = 12d

- Basically, I do the average of the array of spacing [5d, 2d, 10d, 0d, 2d, 5d]
And if daysToPres is bigger than that, I include it in the average,
so that it only stretches the average when a transaction was expected by that time, but was not observed.

- from that average spacing "daysToNext", I compute the density of occurence per day.

#### Decide what changes (if any) need to be made to the $--/day model so that it satisfies the requirements:
- the next section explains what I think might be a compeling solution.

#### Determine formula for how much has been saved for at any time.
- I am going to try to return the following in the json oracle:
"savedFor" = daysToPres / daysToNext = "time from last such trans to present" / "time we expect(ed) this trans should come within".
Indeed, this whole PriceWindow expects something to happen every other "daysToNext". So 1 day after the last transaction,
we have actually saved for 1/daysToNext, 2 days after we have saved for 2/daysToNext, and daysToNext days after, we have saved for 100%.
After that, we have saved for 110%, 120%, etc...
But if you suddenly buy something that falls in this price window, you kinda reset this to 0%,
unless if we had saved for much more than 100%, in wich case, sure, we should buy it anyway.

## PriceWindows vs. Habits
Habits & priceWindows should be thought of as a strong form & a weak form, respectively, of the same feature: both are used to predict the likelihood of a future expense, and this serves 2 purposes:

1. To calculate the total $--/day for a user as accurately as possible.
2. To communicate to the user what is contributing to their $--/day. E.g. "You're averaging $10/day on Uber", or "I'm seeing $50 expenses about once a week these days".

Habits are more accurate than priceWindows. But priceWindows have the benefit of being **onto**: for every transaction there exists a priceWindow, whereas it is not a given that for every transaction there exists a habit.

This means that whenever possible, a transaction's classification within a habit should trump its classification within a priceWindow.

A transaction should also only be classified within one - either a habit or a priceWindow - not both. If it can be found to belong to habit, it should be removed, at least in certain respects, from the priceWindow. This is for the following reason. Habits predict future expenses just as much as priceWindows do, and therefore both should contribute to the user's $--/day. Including a transaction in both a habit and a priceWindow would double-count its contribution to the $--/day.

### Habits vs. PriceWindows are useful for askFlow

Habits are important for answering askFlow questions. Take my $20 BART pass, which I purchase every 3 days, as an example. Hours before purchasing another pass, I might ask Cache about a $20 expense that isn't BART, and it will say yes, that Cache has saved for a $20 expense. Most likely it has saved for the BART pass, and most likely I won't realize this. That is because I'm not really aware of BART passes until I try to enter BART and realize it needs to be filled. So I'll have been advised about a single $20 today, when in reality I made 2. If Cache-Bot has correctly identified the BART pass as a habit, it can offer a richer answer to my askFlow question, e.g.:
>If it's BART-CLIPPER, I've saved for it. Otherwise, a $20 expense today would increase your spending by $x/day for a couple days.

This could become complicated on the UX side, if a user has multiple habits intersecting with an askFlow price, such as $20. Regardless of what we communicate to the user, distinguishing between habits and nonhabits will be useful in guessing a best answer for a user.

## Conversation flows
### Prompts
Prompts are notifications sent by Cache, that may or may not include a question (preferably yes/no)
#### Income prompts
### Advice
#### Types
##### Price windows
```
{
    type         : String (e.g. 'window')
    , score      : Number (the average daily difference divded by the time we are noticing this difference over = $/day/day)
                          (It is like amortizing the bad/good behavior)
    , approxAmnt : Number (e.g. 50)
    , factStr    : String (temporary human readable version for debugging)
    , dailyOld   : Number ($/day)
    , dailyNew   : Number ($/day)
    , dailyDif   : Number ($/day)
    , flowEffect : Number (flow change)
    , overDays   : Number (e.g. 7 for a trend7)
}
```
