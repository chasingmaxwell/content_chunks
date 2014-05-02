<?php

/**
 * @file
 * Paragraph chunk template.
 */

/**
 * @TODO: add documentation about variables.
 */

$configuration['p'] = trim(preg_replace('/\s+/', ' ', $configuration['p']));

?>

<p class="<?php print $classes; ?>">
  <?php print check_plain($configuration['p']); ?>
</p>
