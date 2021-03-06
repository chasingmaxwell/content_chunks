<?php

/**
 * @file
 * Chunk template for the full HTML chunk type.
 *
 * Available variables:
 * - $classes: The string value for the class attribute initially generated by
 *   template_preprocess().
 * - $configuration: The associative array representing the chunk's
 *   configuration.
 *   - html: A string of text which contains HTML markup to be run through the
 *     Full HTML input filter.
 * - $instance_settings: An associative array representing the chunk instance
 *   settings
 *   connected to the chunk field's instance.
 * - $chunk_type: The ChunkType object representing the chunk's type.
 * - $entity: If we are viewing an entity, this will be the entity object.
 *   Otherwise it will be FALSE.
 * - $entity_type: If we are viewing an entity, this will be the string
 *   represnting the entity's type. Otherwise it will be FALSE.
 * - $field_name: The field name for the parent chunks field.
 * - $langcode: The language code for the parent chunks field.
 * - $delta: The delta for the current chunk within the parent chunks field.
 * - $preview: A boolean indicating whether or not the chunk is being previewed.
 *
 * @see template_preprocess()
 * @see template_preprocess_chunk()
 *
 * @ingroup themeable
 */
?>

<div class="<?php print $classes; ?>">
  <?php print check_markup($configuration['html'], 'full_html'); ?>
</div>
