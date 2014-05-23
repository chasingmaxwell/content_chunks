<?php

/**
 * @file
 * Chunk template for the list chunk type.
 *
 * Available variables:
 * - $classes: The string value for the class attribute initially generated by
 *   template_preprocess().
 * - $configuration: The associative array representing the chunk's
 *   configuration.
 *   - style: Either 'ul' or 'ol'.
 *   - items: An indexed array of strings to be displayed each in a list item.
 *   - edit_in_place: A boolean indicating whether or not this chunk is being
 *     edited in place.
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
 *
 * @see template_preprocess()
 * @see template_preprocess_chunk()
 *
 * @ingroup themeable
 */
?>

<<?php print $configuration['style']; ?> class="<?php print $classes; ?>"<?php if ($configuration['edit_in_place']): ?> contenteditable<?php endif; ?>>
  <?php foreach ($configuration['list'] as $item): ?>
    <li><?php print check_markup($item, $type_settings['format']); ?></li>
  <?php endforeach; ?>
</<?php print $configuration['style']; ?>>
