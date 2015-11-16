# CacheBot README.md
## Currently super important
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
"percentSaved" = daysToPres / daysToNext = "time from last such trans to present" / "time we expect(ed) this trans should come within".
Indeed, this whole PriceWindow expects something to happen every other "daysToNext". So 1 day after the last transaction, 
we have actually saved for 1/daysToNext, 2 days after we have saved for 2/daysToNext, and daysToNext days after, we have saved for 100%.
After that, we have saved for 110%, 120%, etc...
But if you suddenly buy something that falls in this price window, you kinda reset this to 0%,
unless if we had saved for much more than 100%, in wich case, sure, we should buy it anyway.

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
