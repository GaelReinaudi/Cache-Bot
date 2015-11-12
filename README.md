# CacheBot README.md
## Currently super important
### Yes/no buying decisions
#### Describe here how cache-bot currently determines $--/day total costs for price windows.
#### Decide what changes (if any) need to be made to the $--/day model so that it satisfies the requirements:
#### Determine formula for how much has been saved for at any time.
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
