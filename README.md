# CacheBot README.md
## Conversation flows
### Prompts
Prompts are notifications sent by Cache, that may or may not include a question (preferably yes/no)
### Yes / no buying decisions
### Advice
#### Types
##### Price windows
```
{
    type     : String (e.g. 'window')
    , min    : Number (e.g. 50)
    , max    : Number (e.g. 75)
    , advice : String (constant, e.g. kBeCarefulkGoodJob/kHolyShit)
    , effect : Number (flow rate, dollar amount (per day? per month?))
    , time?  : Number (e.g. num_days until it's over? (doesn't make sense for kGoodJobs?), or #timespermonth, etc.
}
```
