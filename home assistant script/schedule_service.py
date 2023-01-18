# schedule_service.py

# helper function to sort schedule on time order
def sortOnTime(e):
  return e['eventTime']

# main entry
eventList = []
schedule_device = 0
event_count = 0
daysList = []
hours = []
mins = []
actionList = []
day_entity_id = data.get('day_entity_id')
wk_entity_id = data.get('wk_entity_id')
schedule_slot = data.get('schedule_slot')
scheduler_name = data.get('schedule_name')
logger.info('send schedule call')
logger.info('wk %d', wk_entity_id)
# extract weekday schedule
if day_entity_id is not None:
    state = hass.states.get(day_entity_id)
    days = state.attributes.get('weekdays') or 'none'
    times = state.attributes.get('timeslots') or 'none'
    actions = state.attributes.get('actions') or 'none'
    
    for day in days:
      action_cnt = 0
      for time in times:
        actionStr = list(actions[action_cnt].values())[0:1][0]
        
        actionStr = actionStr.split('_')[2]
        if actionStr == 'off':
          action = 0
        else:
          action = 1
        time = time.split(' - ')[0]
        day = day.replace('sun', '1').replace( 'mon', '2').replace( 'tue', '3').replace( 'wed', '4',).replace('thu', '5').replace( 'fri', '6').replace( 'sat', '7')
        sched_item = day + ':' + time + '-'
        eventList.append({"eventTime":sched_item,"eventAction":  action})
        action_cnt =  action_cnt+ 1
        
else:
  logger.error('day enity empty')
 
# extract weekend schedule
if wk_entity_id is not None:
    state = hass.states.get(wk_entity_id)
    days = state.attributes.get('weekdays') or 'none'
    times = state.attributes.get('timeslots') or 'none'
    actions = state.attributes.get('actions') or 'none'
    for day in days:
      action_cnt = 0
      for time in times:
        actionStr = list(actions[action_cnt].values())[0:1][0]
        actionStr = actionStr.split('_')[2]
        if actionStr == 'off':
          action = 0
        else:
          action = 1
        time = time.split(' - ')[0]
        day = day.replace('sun', '1').replace( 'mon', '2').replace( 'tue', '3').replace( 'wed', '4',).replace('thu', '5').replace( 'fri', '6').replace( 'sat', '7')
        sched_item = day + ':' + time + '-'
        eventList.append({"eventTime":sched_item,"eventAction":  action})
        action_cnt =  action_cnt+ 1
        
else:
  logger.error('weekend enity empty')
# sort the schedule list by time
eventList.sort(key=sortOnTime)

event_count = 0
prev_event = 0
for event in eventList:
      eventTime = event["eventTime"]
      if event_count == 0:
        day = eventTime.split(':')[0]
        daysList.append(int(day))
        min = eventTime.split(':')[2]
        hour  = eventTime.split(':')[1]
        hours.append(int(hour))
        mins.append(int(min))
        actionList.append( event["eventAction"])
        prev_event =  event["eventAction"]
        event_count = event_count +1
      else:
        if  event["eventAction"] !=  prev_event:
          day = eventTime.split(':')[0]
          daysList.append(int(day))
          min = eventTime.split(':')[2]
          hour  = eventTime.split(':')[1]
          hours.append(int(hour))
          mins.append(int(min))
          actionList.append( event["eventAction"])
          prev_event =  event["eventAction"]
          event_count = event_count +1
logger.info('Sending %d events', len(actionList))

if actionList is not None:
   # hass.services.call('esphome', 'boiler_controller_update_schedule', {'schedule_string':daySchedule })
   hass.services.call('esphome', 'schedule_test_send_schedule',
                      {'scheduler_id': scheduler_name, 'schedule_slot_id': int(schedule_slot),  'days': daysList, 'hours': hours, 'minutes': mins, 'actions': actionList})
    
