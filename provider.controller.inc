<?php

/**
 * @file
 *
 * Controller for loading, creating, and saving providers.
 *
 * The default loader, which we extend, handles load() already. We only
 * need to add saving and creating.
 */
class ProviderController extends DrupalDefaultEntityController {

  /**
   * Creates a transaction to save a provider and a revision.
   *
   * @param object $provider
   *   The fully loaded provider object.
   *
   * @return object
   *   The saved provider.
   */
  public function save($provider) {
    $transaction = db_transaction();

    try {
      global $user;

      // Determine if we will be inserting a new provider.
      $provider->is_new = empty($provider->pid);

      // Set the timestamp fields.
      if (empty($provider->created)) {
        $provider->created = REQUEST_TIME;
      }
      $provider->changed = REQUEST_TIME;

      $provider->revision_timestamp = REQUEST_TIME;
      $update_provider = TRUE;

      // Give modules the opportunity to prepare field data for saving.
      field_attach_presave('provider', $provider);

      // When saving a new provider revision, unset any existing $provider->vid
      // to ensure a new revision will actually be created and store the old
      // revision ID in a separate property for provider hook implementations.
      if (!$provider->is_new && !empty($provider->revision) && $provider->vid) {
        $provider->old_vid = $provider->vid;
        unset($provider->vid);
      }

      // If this is a new provider...
      if ($provider->is_new) {
        // Save the new provider.
        drupal_write_record('provider', $provider);

        // Save the initial revision.
        $this->saveRevision($provider, $user->uid);

        $op = 'insert';
      }
      else {
        // Save the updated provider.
        drupal_write_record('provider', $provider, 'pid');

        // If a new provider revision was requested, save a new record for that;
        // otherwise, update the provider revision record that matches the value
        // of $provider->vid.
        if (!empty($provider->revision)) {
          $this->saveRevision($provider, $user->uid);
        }
        else {
          $this->saveRevision($provider, $user->uid, TRUE);
          $update_provider = FALSE;
        }

        $op = 'update';
      }

      // If the revision ID is new or updated, save it to the provider.
      if ($update_provider) {
        db_update('provider')
          ->fields(array('vid' => $provider->vid))
          ->condition('pid', $provider->pid)
          ->execute();
      }

      // Save fields.
      $function = 'field_attach_' . $op;
      $function('provider', $provider);

      module_invoke_all('entity_' . $op, $provider, 'provider');

      // Clear internal properties.
      unset($provider->is_new);

      // Ignore slave server temporarily to give time for the saved order to be
      // propagated to the slave.
      db_ignore_slave();

      // Fetch items for this provider.
      provider_update_provider();

      return $provider;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('provider', $e, NULL, WATCHDOG_ERROR);
      return FALSE;
    }
  }

  /**
   * Saves a provider revision with the uid of the current user.
   *
   * @param object $provider
   *   The fully loaded provider object.
   * @param int $uid
   *   The user's uid for the current revision.
   * @param bool $update
   *   TRUE or FALSE indicating whether or not the existing revision should be
   *     updated instead of a new one created.
   */
  protected function saveRevision($provider, $uid, $update = FALSE) {
    // Hold on to the provider's original creator_uid but swap in the revision's
    // creator_uid for the momentary write.
    $temp_uid = $provider->uid;
    $provider->uid = $uid;

    // Update the existing revision if specified.
    if ($update) {
      drupal_write_record('provider_revision', $provider, 'vid');
    }
    else {
      // Otherwise insert a new revision. This will automatically update
      // $provider to include the vid.
      drupal_write_record('provider_revision', $provider);
    }

    // Reset the order's creator_uid to the original value.
    $provider->uid = $temp_uid;
  }

  /**
   * Deletes multiple providers by ID.
   *
   * @param array $pids
   *   An array of provider IDs to delete.
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function delete($pids) {
    if (!empty($pids)) {
      $providers = $this->load($pids, array());

      $transaction = db_transaction();

      try {
        db_delete('provider')
          ->condition('pid', $pids, 'IN')
          ->execute();

        db_delete('provider_revision')
          ->condition('pid', $pids, 'IN')
          ->execute();

        foreach ($providers as $provider_id => $provider) {
          field_attach_delete('provider', $provider);
        }

        // Ignore slave server temporarily to give time for the
        // saved provider to be propagated to the slave.
        db_ignore_slave();
      }
      catch (Exception $e) {
        $transaction->rollback();
        watchdog_exception('provider', $e, NULL, WATCHDOG_ERROR);
        return FALSE;
      }

      module_invoke_all('entity_delete', $provider, 'provider');

      // Clear the page and block and provider caches.
      cache_clear_all();
      $this->resetCache();
    }

    return TRUE;
  }

  /**
   * Create a default provider.
   *
   * @param string $type
   *   The machine-readable type of the provider.
   *
   * @return object
   *   An provider object with all default fields initialized.
   */
  public function create($type = '') {
    return (object) array(
      'pid' => '',
      'type' => $type,
      'title' => '',
    );
  }
}
