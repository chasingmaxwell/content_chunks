<?php

/**
 * @file
 * Provide hooks relevant to the image chunk type.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_image_chunk_form($form, &$form_state, &$configuration) {
  $config_form = array();
  // @TODO: figure out what properties the image's widget form needs to operate
  // properly and provide them.
  // @TODO: load the image's widget form.
  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_image_chunk_form_validate($configuration, &$form_state, $form) {
  // @TODO: run whatever validation the image supplies.
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_image_chunk_is_empty($configuration, $item, $field) {
  // @TODO: check if the image has it's own empty check and run it.
}
