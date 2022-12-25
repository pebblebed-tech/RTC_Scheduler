# schedule_service.py
daySchedule = []
day_entity_id = data.get('day_entity_id')
wk_entity_id = data.get('wk_entity_id')
logger.info('send schedule called')
if day_entity_id is not None:
    state = hass.states.get(day_entity_id)
    days = state.attributes.get('weekdays') or 'none'
    times = state.attributes.get('timeslots') or 'none'
    actions = state.attributes.get('actions') or 'none'
    for day in days:
      action_cnt = 0
      for time in times:
        action = list(actions[action_cnt].values())[0:1][0]
        action = action.split('_')[1]
        time = time.split(' - ')[0]
        day = day.replace('sun', '1').replace( 'mon', '2').replace( 'tue', '3').replace( 'wed', '4',).replace('thu', '5').replace( 'fri', '6').replace( 'sat', '7')
        sched_item = day + ':' + time + '-' + action +','
        daySchedule.append(sched_item)
        action_cnt =  action_cnt+ 1
if wk_entity_id is not None:
    state = hass.states.get(wk_entity_id)
    days = state.attributes.get('weekdays') or 'none'
    times = state.attributes.get('timeslots') or 'none'
    actions = state.attributes.get('actions') or 'none'
    for day in days:
      action_cnt = 0
      for time in times:
        action = list(actions[action_cnt].values())[0:1][0]
        action = action.split('_')[1]
        time = time.split(' - ')[0]
        day = day.replace('sun', '1').replace( 'mon', '2').replace( 'tue', '3').replace( 'wed', '4',).replace('thu', '5').replace( 'fri', '6').replace( 'sat', '7')
        sched_item = day + ':' + time + '-' + action + ','
        daySchedule.append(sched_item)
        action_cnt =  action_cnt+ 1
logger.info('Sending %s', daySchedule)

if daySchedule is not None:
    hass.services.call('esphome', 'boiler_controller_update_schedule', {'schedule_string':daySchedule })
