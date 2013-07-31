<?php

/**
 * @file
 * Creates admin pages.
 */

/**
 * List all providers of the specified type and provide some helpful links.
 */
function provider_list() {
  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => '3'));
  $rows = array();
  $query = new EntityFieldQuery();
  $entity = $query->entityCondition('entity_type', 'provider')
    ->entityCondition('type', arg(4));
  $result = $query->execute();

  if ($result) {
    foreach ($result as $entity) {
      $entity = reset($entity);
      $provider = provider_load(intval($entity->pid));

      $row[] = array('data' => $provider->title);
      $row[] = array('data' => l(t('view'), 'provider/' . $provider->pid));
      $row[] = array('data' => l(t('edit'), 'provider/' . $provider->pid . '/edit'));
      $row[] = array('data' => l(t('update'), 'provider/' . $provider->pid . '/update'));
      $rows[] = $row;
    }
  }

  $build['provider_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => 'There are no providers.',
  );

  return $build;
}

/**
 * Displays a provider.
 *
 * @param object $provider
 *   The provider object to display.
 * @param string $view_mode
 *   The view mode we want to display.
 */
function provider_page_view($provider, $view_mode = 'full') {
  // Remove previously built content, if exists.
  $provider->content = array();

  if ($view_mode == 'teaser') {
    $provider->content['title'] = array(
      '#markup' => filter_xss($provider->title),
      '#weight' => -5,
    );
  }

  // Build fields content.
  field_attach_prepare_view('provider', array($provider->pid => $provider), $view_mode);
  entity_prepare_view('provider', array($provider->pid => $provider));
  $provider->content += field_attach_view('provider', $provider, $view_mode);

  return $provider->content;
}


/**
 * Menu callback; List all provider types available.
 */
function provider_overview_types() {
  $types = provider_types();
  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => '5'));
  $rows = array();

  foreach ($types as $type) {
    $type_url_str = str_replace('_', '-', $type->type);
    $row[] = array('data' => $type->name);
    $row[] = array('data' => l(t('list'), 'admin/structure/providers/manage/' . $type_url_str . '/list'));
    $row[] = array('data' => l(t('add'), 'provider/add/' . $type_url_str));
    $row[] = array('data' => l(t('manage fields'), 'admin/structure/providers/manage/' . $type_url_str . '/fields'));
    $row[] = array('data' => l(t('manage display'), 'admin/structure/providers/manage/' . $type_url_str . '/display'));
    $row[] = array('data' => l(t('delete'), 'admin/structure/providers/manage/' . $type_url_str . '/delete'));
    $rows[] = $row;
  }

  $build['provider_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Menu callback; Show list of provider types we can add.
 */
function provider_add_page() {
  $types = provider_types();
  $items = array();

  foreach ($types as $type) {
    array_push($items, l($type->name, "provider/add/" . $type->type));
  }

  return theme('item_list', array('items' => $items, 'title' => 'Available Providers'));
}

/**
 * Menu callback; Provider information page.
 *
 * @param object $provider_type
 *   The type of provider.
 */
function provider_information($provider_type) {
  return $provider_type->name . ': ' . $provider_type->description;
}

/**
 * Menu callback; shows the provider edit form or redirects to delete.
 *
 * @param object $provider
 *   The provider object to edit.
 */
function provider_page_edit($provider) {
  $types = provider_types();
  drupal_set_title(t('<em>Edit @type</em> @title', array('@type' => $types[$provider->type]->name, '@title' => $provider->title)), PASS_THROUGH);

  return drupal_get_form('provider_form', $provider);
}

/**
 * The provider submission form.
 */
function provider_add($type) {
  global $user;

  $types = provider_types();
  $type = isset($type) ? str_replace('-', '_', $type) : NULL;
  if (empty($types[$type])) {
    return MENU_NOT_FOUND;
  }

  $provider = entity_get_controller('provider')->create($type);

  drupal_set_title(t('Create @name', array('@name' => $types[$type]->name)), PASS_THROUGH);
  return drupal_get_form('provider_form', $provider);
}

/**
 * Form builder; Displays the provider add/edit form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 * @param object $provider
 *   The provider object to edit, which may be brand new.
 *
 * @return array
 *   The altered $form.
 */
function provider_form($form, &$form_state, $provider) {
  // Set the id and identify this as an provider edit form.
  $form['#id'] = 'provider-form';

  // Save the provider for later, in case we need it.
  $form['#provider'] = $provider;
  $form_state['provider'] = $provider;

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $provider->title,
    '#weight' => -4,
    '#required' => TRUE,
  );
  $form['revision'] = array(
    '#access' => user_access('administer providers'),
    '#type' => 'checkbox',
    '#title' => t('Create new revision'),
    '#default_value' => 0,
  );

  // Add the buttons.
  $form['buttons'] = array();
  $form['buttons']['#weight'] = 100;
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('provider_form_submit'),
  );
  if (!empty($provider->pid)) {
    $form['buttons']['delete'] = array(
      '#access' => user_access('delete providers'),
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('provider_form_delete_submit'),
    );
  }

  $form['#validate'][] = 'provider_form_validate';

  field_attach_form('provider', $provider, $form, $form_state);

  return $form;
}

/**
 * Form bulder; Validate form submission.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function provider_form_validate($form, &$form_state) {
  $provider = $form_state['provider'];

  // Field validation.
  field_attach_form_validate('provider', $provider, $form, $form_state);
}

/**
 * Form bulder; Submit the provider.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function provider_form_submit($form, &$form_state) {
  global $user;

  $provider = &$form_state['provider'];

  // Set the provider's uid if it's being created at this time.
  if (empty($provider->uid)) {
    $provider->uid = $user->uid;
  }

  $provider->title = $form_state['values']['title'];
  $provider->revision = $form_state['values']['revision'];

  // Notify field widgets.
  field_attach_submit('provider', $provider, $form, $form_state);

  // Save the provider.
  provider_save($provider);

  // Notify the user.
  drupal_set_message(t('Provider saved.'));

  $form_state['redirect'] = 'provider/' . $provider->pid;
}

/**
 * Form bulder; Delete a provider.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function provider_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $provider = $form['#provider'];
  $form_state['redirect'] = array('provider/' . $provider->pid . '/delete', array('query' => $destination));
}

/**
 * Form bulder; Asks for confirmation of provider deletion.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 * @param object $provider
 *   The provider object to edit, which may be brand new.
 */
function provider_delete_confirm($form, &$form_state, $provider) {
  $form['#provider'] = $provider;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['pid'] = array('#type' => 'value', '#value' => $provider->pid);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $provider->title)),
    'provider/' . $provider->pid,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Executes provider deletion.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function provider_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $provider = provider_load(intval($form_state['values']['pid']));
    provider_delete($form_state['values']['pid']);
    watchdog('provider', '@type: deleted %title.', array('@type' => $provider->type, '%title' => $provider->title));

    $types = provider_types();
    drupal_set_message(t('@type %title has been deleted.', array('@type' => $types[$provider->type]->name, '%title' => $provider->title)));
  }

  $form_state['redirect'] = '<front>';
}
