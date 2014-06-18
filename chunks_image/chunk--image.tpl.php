<?php

/**
 * @file
 * Chunk template for the image chunk type.
 *
 * Available variables:
 * - $classes: The string value for the class attribute initially generated by
 *   template_preprocess().
 * - $configuration: The associative array representing the chunk's
 *   configuration.
 * - $type_settings: An associative array representing the chunk type settings
 *   connected to the chunk field's instance.
 * - $chunk_type: The ChunkType object representing the chunk's type.
 * - $entity: If we are viewing an entity, this will be the entity object.
 *   Otherwise it will be FALSE.
 * - $entity_type: If we are viewing an entity, this will be the string
 *   represnting the entity's type. Otherwise it will be FALSE.
 * - $field_name: The field name for the parent chunks field.
 * - $langcode: The language code for the parent chunks field.
 * - $delta: The delta for the current chunk within the parent chunks field.
 * - $image: A rendereable array for the image to be displayed for this chunk.
 *
 * @see template_preprocess()
 * @see template_preprocess_chunk()
 * @see chunks_image_preprocess_chunk()
 *
 * @ingroup themeable
 */
?>

<div class="<?php print $classes; ?>">
  <?php print render($image); ?>
</div>

