<?php

/**
 * @file
 * Paragraph chunk template.
 */

/**
 * @TODO: add documentation about variables.
 */

?>

<p class="<?php print $classes; ?>" <?php if (isset($configuration['editor'])): ?>contenteditable="true"<?php endif; ?>>
  <?php print filter_xss(trim(preg_replace('/(\s|<br>|<\/?div>)+/', ' ', $configuration['p'])), array('a', 'b', 'em', 'i', 'strong', 'u')); ?>
</p>
